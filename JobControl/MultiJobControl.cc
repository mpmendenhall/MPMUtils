/// \file MultiJobControl.cc
// Michael P. Mendenhall, LLNL 2019

#include "MultiJobControl.hh"

MultiJobControl* MultiJobControl::JC = nullptr;

void JobSpec::display() const {
    printf("JobSpec [Job %i: %zu -- %zu] for class '%s' on worker [%i]\n", uid, N0, N1, ObjectFactory::nameOf(wclass).c_str(), wid);
}

void JobComm::splitJobs(vector<JobSpec>& vJS, size_t nSplit, size_t nItms, size_t wclass, int uid) {
    for(size_t i=0; i<nSplit; i++) {
        vJS.emplace_back();
        vJS.back().uid = uid;
        vJS.back().wclass = wclass;
        vJS.back().N0 = (nItms*i)/nSplit;
        vJS.back().N1 = (nItms*(i+1))/nSplit;
        vJS.back().C = this;
    }
}

REGISTER_FACTORYOBJECT(JobWorker)

void JobWorker::run(JobSpec JS, BinaryIO&) {
    printf("JobWorker does nothing for ");
    JS.display();
}

void MultiJobControl::runWorker() {
    map<size_t, JobWorker*> workers;

    do {
        dataSrc = parentRank;
        auto JS = receive<JobSpec>();

        // null worker type indicates end of run
        if(!JS.wclass) {
            if(verbose > 2) printf("Break command received by [%i]\n", rank);
            break;
        }

        // load and run appropriate worker
        auto it = workers.find(JS.wclass);
        auto W = it == workers.end()? nullptr : it->second;
        if(!W) {
            if(verbose > 3) printf("Instantiating worker class '%s'.\n", ObjectFactory::nameOf(JS.wclass).c_str());
            it->second = W = dynamic_cast<JobWorker*>(ObjectFactory::construct(JS.wclass));
            if(!W) exit(44);
        } else if(verbose > 4) printf("Already have worker class '%s'.\n", ObjectFactory::nameOf(JS.wclass).c_str());
        W->run(JS, *this);

    } while(persistent);
    if(verbose > 2 && !persistent) printf("\nrunWorker completed on [%i]\n\n", rank);

    // cleanup
    for(auto& kv: workers) delete kv.second;
}

int MultiJobControl::submitJob(JobSpec& JS) {
    dataSrc = dataDest = JS.wid = _allocWorker();
    if(verbose > 4) { printf("Submitting "); JS.display(); }
    send(JS);
    if(JS.C) JS.C->startJob(*this);
    jobs[JS.wid] = JS;
    return JS.wid;
}

bool MultiJobControl::isRunning(int wid) {
    auto it = jobs.find(wid);
    if(it == jobs.end()) return false;
    if(_isRunning(wid)) return true;

    auto C = it->second.C;
    jobs.erase(it);
    dataSrc = dataDest = wid;
    if(C) C->endJob(*this);
    clearOut();
    clearIn();
    return false;
}

int MultiJobControl::checkJobs() {
    vector<int> wids;
    for(auto& kv: jobs) wids.push_back(kv.first);
    int nrunning = 0;
    for(auto wid: wids) nrunning += isRunning(wid);
    return nrunning;
}

void MultiJobControl::waitComplete() {
    int i = 0;
    while((i = checkJobs())) {
        if(verbose > 4) printf("Waiting for %i jobs to complete.\n", i);
        usleep(1000000);
    }
}

void MultiJobControl::waitFor(const vector<int>& v) {
    vector<int> wids = v;
    while(wids.size()) {
        vector<int> stillgoing;
        for(auto wid: wids) if(isRunning(wid)) stillgoing.push_back(wid);
        if(!stillgoing.size()) break;
        wids = stillgoing;
        //checkJobs();
        usleep(1000000);
    }
}
