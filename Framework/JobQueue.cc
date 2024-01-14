/// @file JobQueue.cc

#include "JobQueue.hh"
#include <algorithm>

JobQueue::Job JobQueue::jthread::haltThread;

JobQueue::jthread::jthread(JobQueue& jq): JQ(jq) {
    pthread_create(&t, NULL, &jqworkthread, this);
}

void JobQueue::jthread::launch(Job* j) {
    std::unique_lock<std::mutex> lk(jlock);
    jready.wait(lk, [=]{ return !jToRun; });
    jToRun = j;
    jready.notify_one();
}

JobQueue::jthread::~jthread() {
    launch(&haltThread);
    pthread_join(t, nullptr);
}

void* JobQueue::jthread::jqworkthread(void* vJT) {
    ((jthread*)vJT)->doWork();
    return nullptr;
}

void JobQueue::jthread::doWork() {
    if(JQ.verbose) printf("Starting worker %p thread.\n", (void*)this);

    while(true) {
        // wait for job to run
        if(JQ.verbose > 1) { printf("Worker %p awaiting job.\n", (void*)this); fflush(stdout); }
        Job* J = nullptr;
        {
            // acquire unique_lock on queue in this scope
            std::unique_lock<std::mutex> lk(jlock);
            // unlock; wait until condition true; re-lock
            jready.wait(lk, [this]{ return this->jToRun; });
            std::swap(J,jToRun);
            jready.notify_one();
        }
        assert(J);

        if(J == &haltThread) break;

        // run the job
        auto qn = J->qn;
        if(JQ.verbose > 1) { printf("Worker %p running job %p from queue %i\n", (void*)this, (void*)J, qn); fflush(stdout); }
        J->run();
        if(JQ.verbose > 1) { printf("Worker %p completed job %p from queue %i\n", (void*)this, (void*)J, qn); fflush(stdout); }

        // decrement queue worker count
        {
            std::unique_lock<std::mutex> lk(JQ.jqsLock);
            JQ.jqs[qn].n_workers--;
            JQ.v_jdone.notify_all();
            JQ.v_jnew.notify_all();
        }

        // return worker thread to idle pool
        {
            std::unique_lock<std::mutex> lk(JQ.idleLock);
            JQ.j_idle.push_back(this);
            JQ.wready.notify_all();
        }
    }

    if(JQ.verbose) printf("Stopping worker thread %lu.\n", t);
}

void JobQueue::launch(size_t nw) {
    if(cThrd) return; // already running
    halt = false;
    nworkers = nw;
    pthread_create(&cThrd, NULL, &jqcontrolthread, this);
}

void JobQueue::setQueue(int qn, size_t max_workers, size_t backlog) {
    std::unique_lock<std::mutex> lk(jqsLock);
    auto& q = jqs[qn];
    q.max_workers = max_workers;
    q.backlog = backlog;
}

void JobQueue::add(Job* J) {
    if(!J) return;
    std::unique_lock<std::mutex> lk(jqsLock);
    auto& q = jqs[J->qn];
    if(verbose > 4) printf("Adding job to queue %i (backlog %zu)\n", J->qn, q.js.size());
    v_jdone.wait(lk, [&]{ return J->qn || q.js.size() < q.backlog; });
    q.js.push_back(J);
    nwaiting++;
    v_jnew.notify_all();
}

void JobQueue::flush() {
    if(verbose) { printf("Flushing "); display(); }
    std::unique_lock<std::mutex> lk(idleLock);
    wready.wait(lk, [&]{return !nwaiting && j_idle.size() == nworkers;});
}

void JobQueue::display() {
    std::unique_lock<std::mutex> lk(jqsLock);
    printf("JobQueue with %zu pending jobs, %zu/%zu idle workers:\n",
           nwaiting, j_idle.size(), nworkers);
    for(auto& kv: jqs) {
        auto& q = kv.second;
        printf("\tQueue %i: running %zu/%zu workers, backlog %zu/%zu.\n",
               kv.first, q.n_workers, q.max_workers, q.js.size(), q.backlog);
    }
}

void JobQueue::shutdown() {
    if(halt) return;

    flush();
    if(verbose) printf("Shutting shown controller thread.\n");
    halt = true;
    v_jnew.notify_all();
    if(cThrd) pthread_join(cThrd, nullptr);
    cThrd = 0;
}

JobQueue::jthread* JobQueue::getIdle() {
    if(verbose > 2) printf("Waiting for idle thread.\n");
    std::unique_lock<std::mutex> lk(idleLock);
    wready.wait(lk, [&]{return j_idle.size();});
    auto JT = j_idle.back();
    j_idle.pop_back();
    if(verbose > 2) printf("Idle thread %p available.\n", (void*)JT);
    return JT;
}

void* JobQueue::jqcontrolthread(void* vJQ) {
    reinterpret_cast<JobQueue*>(vJQ)->runController();
    return nullptr;
}

JobQueue::jqueue* JobQueue::chooseNext() {
    nextQ = nullptr;
    size_t blmax = 0;
    for(auto& kv: jqs) {
        if(kv.second.n_workers >= kv.second.max_workers) {
            if(verbose > 3) printf("\tQueue %i at max %zu workers.\n", kv.first, kv.second.n_workers);
            continue;
        }
        if(kv.second.js.size() > blmax) {
            nextQ = &kv.second;
            blmax = nextQ->js.size();
        }
    }
    if(verbose > 2) printf("Selected next queue %p\n", (void*)nextQ);
    return nextQ;
}

void JobQueue::runController() {
    // spin up worker threads
    if(!nworkers) return;
    while(j_idle.size() < nworkers) j_idle.push_back(new jthread(*this));
    auto vjall = j_idle;

    while(true) {
        Job* nextUp = nullptr;
        {
            if(verbose > 3) printf("Controller finding next job...\n");

            std::unique_lock<std::mutex> lk(jqsLock);
            v_jnew.wait(lk, [=]{ return halt || chooseNext(); });
            if(halt) break;

            assert(nextQ->js.size());
            nextUp = nextQ->js.front();
            nextQ->js.pop_front();
            nextQ->n_workers++;
            nwaiting--;
        }
        assert(nextUp);
        getIdle()->launch(nextUp);
    }

    // close down worker threads
    for(auto j: vjall) delete j;
    j_idle.clear();
}
