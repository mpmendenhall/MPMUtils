/// \file MultiProcess.cc

#include "MultiProcess.hh"

REGISTER_FACTORYOBJECT(MultiProcess,ConfigProcess)

void MultiProcess::postconfig(const Setting& S) {
    int nthreads = 1;
    S.lookupValue("nthreads", nthreads);
    nworkers = nthreads;

    int cn = 0;
    for(auto c: children) {
        setQueue(cn, c->threadsafe()? nworkers : 1, cn? 100 : 10);
        cn++;
    }

    JobQueue::verbose = ConfigProcess::verbose;
    _keepsframe = true;
}

void MultiProcess::MPJob::run() {
    FrameSink* c;
    do {
        c = MP->children[qn++];
        c->receive(*F,*MP);
        if(qn == (int)MP->children.size() || F->drop) {
            if(!c->keepsframe()) MP->finished(*F,*c);
            return;
        }
    } while(!c->keepsframe());
}

void MultiProcess::finished(DataFrame& F, FrameSink&) {
    // mark job done; handle next processing step in main process
    MPJob* J;
    {
        std::unique_lock<std::mutex> lk(jsLock);
        auto it = jsteps.find(&F);
        assert(it != jsteps.end());
        J = it->second;
        if(F.drop) J->qn = children.size();
        if(J->qn == (int)children.size()) jsteps.erase(it);
    }

    {
        std::unique_lock<std::mutex> lk(jdoneLock);
        jdone.push_back(J);
        jdonev.notify_one();
    }
}

void MultiProcess::run_pipeline() {
    vector<MPJob*> vd;
    {
        std::unique_lock<std::mutex> lk(jdoneLock);
        vd = jdone;
        jdone.clear();
    }

    for(auto J: vd) {
        if(J->qn < (int)children.size()) add(J);
        else {
            J->F->release();
            J->FS->finished(*J->F,*this);
            jpool.push_back(J);
        }
    }
}

void MultiProcess::receive(DataFrame& F, FrameSource& S) {
    if(!children.size()) {
        S.finished(F,*this);
        return;
    }

    run_pipeline();

    F.claim();
    if(!jpool.size()) jpool.push_back(new MPJob(this));
    auto J = jpool.back();
    jpool.pop_back();
    J->qn = 0;
    J->F = &F;
    J->FS = &S;
    {
        std::unique_lock<std::mutex> lk(jsLock);
        jsteps.emplace(&F, J);
    }
    add(J);
}

void MultiProcess::flush() {
    do {
        run_pipeline();
        JobQueue::flush();
    } while(jdone.size());
}

void MultiProcess::start_data(DataFrame& F) {
    ConfigProcess::start_data(F);
    launch(nworkers);
}

void MultiProcess::end_data(DataFrame& F) {
    F.claim();
    flush();
    for(auto c: children) {
        c->end_data(F);
        flush();
    }
    F.release();
    shutdown();
    assert(!jsteps.size());
}
