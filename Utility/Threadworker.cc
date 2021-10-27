/// \file Threadworker.cc

#include "Threadworker.hh"
#include "TermColor.hh"
#include <time.h>
#include <cmath>
#include <signal.h>

thread_local int _thread_id = -1;

int Threadworker::thread_id() { return _thread_id; }

Threadworker::Threadworker(int i, ThreadManager* m):
worker_id(i), myManager(m) { }

Threadworker::~Threadworker() {
    if(verbose > 3) {
        printf(TERMFG_BLUE "Deleting Threadworker [%i] in state %i" TERMSGR_RESET "\n",
               worker_id, runstat);
        fflush(stdout);
    }
    if(runstat && runstat != INDETERMINATE) {
        printf("Warning: thread id %i deleted from thread %i in state %i\n",
               worker_id, _thread_id, runstat);
    }
}

void Threadworker::threadjob() {
    while(true) {
        check_pause();

        unique_lock<mutex> lk(inputMut);  // acquire unique_lock on queue in this scope
        if(runstat == STOP_REQUESTED) break;
        inputReady.wait(lk);  // unlock until notified
    }
}

void Threadworker::run_here() {
    if(verbose > 1) printf(TERMFG_GREEN "Running Threadworker [%i] locally." TERMSGR_RESET "\n", worker_id);
    if(checkRunning()) throw std::logic_error("Double launch attempted");
    runstat = RUNLOCAL;
    threadjob();
    runstat = IDLE;
    if(verbose > 2) printf(TERMFG_RED "Threadworker [%i] completed locally." TERMSGR_RESET "\n", worker_id);
    if(myManager) myManager->notify_thread_completed(this);
}

void* Threadworker::run_Threadworker_thread(void* p) {
    auto w = static_cast<Threadworker*>(p);
    if(w->verbose) printf(TERMFG_GREEN "Threadworker [%i] threadjob started." TERMSGR_RESET "\n", w->worker_id);
    _thread_id = w->worker_id;
    w->threadjob();
    if(w->verbose) printf(TERMFG_RED "Threadworker [%i] threadjob completed." TERMSGR_RESET "\n", w->worker_id);
    if(w->myManager) w->myManager->notify_thread_completed(w);
    return nullptr;
}

void Threadworker::launch_mythread() {
    if(checkRunning()) throw std::logic_error("Double launch attempted");
    runstat = RUNNING;
    auto rc = pthread_create(&mythread, nullptr, run_Threadworker_thread, this);
    if(rc) throw rc;
}

void Threadworker::pause() {
    unique_lock<mutex> lk(inputMut);
    if(runstat != RUNNING) throw std::logic_error("Invalid state for pause");
    runstat = PAUSE_REQUESTED;
    inputReady.notify_one(); // signal to un-block on any other operations
    inputReady.wait(lk, [this]{ return runstat == PAUSED; }); // unlock and wait until pause request reciprocated
}

void Threadworker::check_pause() {
    unique_lock<mutex> lk(inputMut);  // acquire unique_lock on queue in this scope
    if(runstat == PAUSE_REQUESTED) {
        runstat = PAUSED;
        inputReady.notify_one();      // reciprocate pause request
        inputReady.wait(lk, [this] { return runstat == RUNNING; }); // unlock and wait until pause lifted
    }
}

void Threadworker::unpause() {
    if(runstat != PAUSED) throw std::logic_error("Invalid state for unpause");
    runstat = RUNNING;
    inputReady.notify_one();
}

void Threadworker::request_stop() {
    if(verbose > 3) printf(TERMFG_YELLOW "Asking Threadworker [%i] to stop..." TERMSGR_RESET "\n", worker_id);
    if(runstat == IDLE) throw std::logic_error("Attempt to stop in idle state");
    runstat = STOP_REQUESTED;
    inputReady.notify_one(); // notification to catch STOP_REQUESTED
}

void Threadworker::finish_mythread() {
    if(verbose > 2) printf(TERMFG_YELLOW "Threadworker [%i] asked to finish..." TERMSGR_RESET "\n", worker_id);
    request_stop();
    int rc = pthread_join(mythread, nullptr);
    runstat = IDLE;
    if(rc) printf("Warning: thread %i joined with code %i\n", worker_id, rc);
    if(verbose > 2) printf(TERMFG_RED "Threadworker [%i] is finished." TERMSGR_RESET "\n", worker_id);
}

void Threadworker::kill_mythread(double timeout_s) {
    if(verbose > 2) printf(TERMFG_YELLOW "Threadworker [%i] demanded to finish..." TERMSGR_RESET "\n", worker_id);
    request_stop();

    struct timespec ts;
    if(clock_gettime(CLOCK_REALTIME, &ts)) throw std::runtime_error("clock_gettime failed");
    timeout_s += ts.tv_nsec*1e-9;
    ts.tv_nsec = modf(timeout_s, &timeout_s)*1e9;
    ts.tv_sec += timeout_s;

    if(pthread_timedjoin_np(mythread, nullptr, &ts)) {
        myManager = nullptr;
        pthread_kill(mythread, SIGKILL);
        pthread_join(mythread, nullptr);
        runstat = INDETERMINATE;
    } else runstat = IDLE;
}

//-------------------------
//-------------------------


ThreadManager::~ThreadManager() {
    if(nrunning || mythreads.size() || pendingDone.size())
        printf("Warning: ThreadManager deleted with nrunning = %i (%zu Threadworkers), %zu pending done\n",
               nrunning, mythreads.size(), pendingDone.size());
}

void ThreadManager::await_threads_completion() {
    if(verbose)
        printf(TERMFG_GREEN "---- ThreadManager [%i] waiting for %zu jobs to complete. ----" TERMSGR_RESET "\n",
               worker_id, mythreads.size());

    if(runstat == RUNLOCAL) runstat = STOP_REQUESTED;

    while(true) {
        unique_lock<mutex> lk(inputMut);

        if(!nrunning && runstat == STOP_REQUESTED) break;

        if(pendingDone.size()) {
            if(verbose > 2)
                printf(TERMFG_BLUE "ThreadManager purging %zu completed threads." TERMSGR_RESET "\n",
                       pendingDone.size());
            vector<Threadworker*> pdone;
            std::swap(pdone, pendingDone);
            lk.unlock();
            for(auto t: pdone) remove_thread(t);
            continue;
        }

        inputReady.wait(lk);
    }
}

void ThreadManager::threadjob() {
    for(auto t: mythreads) t->launch_mythread();
    await_threads_completion();
}

void ThreadManager::purge_pending() {
    vector<Threadworker*> pdone;
    {
        lock_guard<mutex> lk(inputMut);
        std::swap(pdone, pendingDone);
    }
    for(auto t: pdone) remove_thread(t);
}

void ThreadManager::add_thread(Threadworker* t, bool autoid) {
    if(!t) throw std::logic_error("attempt to add nullptr thread");

    if(autoid) {
        static int wid = 1;
        t->worker_id = wid++;
    }
    if(verbose > 1) printf(TERMFG_GREEN "ThreadManager adding thread [%i]." TERMSGR_RESET "\n", t->worker_id);
    t->myManager = this;

    {
        lock_guard<mutex> lk(inputMut);
        if(mythreads.count(t)) throw std::logic_error("Same worker added twice");
        mythreads.insert(t);
        ++nrunning;
        inputReady.notify_one();
    }
}

void ThreadManager::notify_thread_completed(Threadworker* t) {
    if(!t) throw std::logic_error("nullptr thread completed");
    if(verbose > 3) printf(TERMFG_RED "ThreadManager notified thread [%i] is completed." TERMSGR_RESET "\n", t->worker_id);
    lock_guard<mutex> lk(inputMut);
    pendingDone.push_back(t);
    inputReady.notify_one();
}

void ThreadManager::remove_thread(Threadworker* t) {
    if(!t) throw std::logic_error("attempt to remove nullptr thread");
    if(verbose > 3) printf(TERMFG_YELLOW "ThreadManager removing thread [%i]." TERMSGR_RESET "\n", t->worker_id);
    {
        lock_guard<mutex> lk(inputMut);
        mythreads.erase(t);
        --nrunning;
    }
    if(t->checkRunning()) t->finish_mythread();
    on_thread_completed(t);
}
