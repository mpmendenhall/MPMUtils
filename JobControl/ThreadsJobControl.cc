/// \file ThreadsJobControl.cc
// -- Michael P. Mendenhall, LLNL 2019

#include "ThreadsJobControl.hh"
#include <thread>

/*
static vector<int> stillRunning;

void ThreadsJobControl::init(int argc, char **argv) {
    exec_name = argv[0];
    DiskIOJobControl::init(argc, argv);

    ntasks = std::max(1, (int)std::thread::hardware_concurrency());
    if(!rank && verbose) printf("ThreadsJobControl running on %i cores.\n", ntasks);
    if(rank && verbose > 2) printf("ThreadsJobControl running on core %i.\n", rank);
    stillRunning.resize(ntasks+1);
}

bool ThreadsJobControl::_isRunning(int wid) {
    auto it = cthreads.find(wid);
    if(it == cthreads.end()) {
        if(verbose > 4) printf("Worker ID %i not in threads list.\n", wid);
        return false;
    }

    if(stillRunning[it->first]) {
        if(verbose > 4) printf("Worker ID %i is still running.\n", wid);
        return true;
    }

    if(verbose > 4) printf("Assuring worker thread %i is done.\n", wid);
    void* ret = nullptr;
    pthread_join(it->second, &ret);
    cthreads.erase(it);
    int iret = size_t(ret);
    if(iret) {
        printf("**** Worker %i exited on code '%i'!\n", wid, iret);
        exit(iret);
    }
    if(verbose > 4) printf("\tWorker thread %i is indeed done.\n", wid);
    return false;
}

pthread_barrier_t runcmdInit;

struct runcmd_args {
    string s;
    int r;
};

void* runcmd(void* s) {
    runcmd_args a = *(runcmd_args*)s;
    stillRunning[a.r] = 1;
    pthread_barrier_wait(&runcmdInit); // release lock on arguments
    runSysCmd(a.s);
    stillRunning[a.r] = 0;
    return nullptr;
}

int ThreadsJobControl::_allocWorker() {
    while((int)checkJobs().size() == ntasks) usleep(10000); // wait for a slot

    for(int i=1; i<=ntasks; i++) {
        if(cthreads.count(i)) continue;

        std::stringstream jcmd;
        jcmd << exec_name << " -N " << i;

        pthread_barrier_init(&runcmdInit, nullptr, 2);

        pthread_t vthread;
        runcmd_args a;
        a.s = jcmd.str();
        a.r = i;
        pthread_create(&vthread, NULL, &runcmd, (void*)&a);

        pthread_barrier_wait(&runcmdInit);
        pthread_barrier_destroy(&runcmdInit);
        cthreads[i] = vthread;

        return i;
    }

    exit(1); // should have a slot available???
}
*/
