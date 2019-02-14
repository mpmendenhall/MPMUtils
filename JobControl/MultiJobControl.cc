/// \file MultiJobControl.cc
// Michael P. Mendenhall, LLNL 2019

#include "MultiJobControl.hh"
#include <algorithm>

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
    MultiJobControl::JC->signalDone();
}

void MultiJobControl::runWorker() {
    map<size_t, JobWorker*> workers;

    do {
        dataDest = dataSrc = parentRank;
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

vector<int> MultiJobControl::checkJobs() {
    vector<int> wids;
    for(auto& kv: jobs) wids.push_back(kv.first);
    wids.erase(std::remove_if(wids.begin(), wids.end(), [&](int i){return !isRunning(i);}), wids.end());
    return wids;
}

void MultiJobControl::waitComplete() {
    vector<int> js;
    while((js = checkJobs()).size()) {
        if(verbose > 4) {
            printf("Waiting for job%s ", js.size() > 1? "s" : "");
            for(auto i: js) printf("%i ",i);
            printf("to complete.\n");
        }
        usleep(verbose > 4? 1000000 : 10000);
    }
}

void MultiJobControl::waitFor(const vector<int>& v) {
    vector<int> wids = v;
    while(wids.size()) {
        wids.erase(std::remove_if(wids.begin(), wids.end(), [&](int i){return !isRunning(i);}), wids.end());
        if(!wids.size()) break;
        //checkJobs();
        usleep(verbose > 4? 1000000 : 10000);
    }
}
