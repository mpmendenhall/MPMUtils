/// \file ThreadsJobControl.cc
// Michael P. Mendenhall, LLNL 2019

#include "ThreadsJobControl.hh"
#include <sstream>
#include <unistd.h>
#include <cstdlib> // for system(...)
#include <thread>

static vector<int> stillRunning;

void ThreadsJobControl::init(int argc, char **argv) {
    exec_name = argv[0];
    DiskIOJobControl::init(argc, argv);

    ntasks = coresPerNode = std::thread::hardware_concurrency();
    if(!ntasks) ntasks = coresPerNode = 1;
    if(!rank && verbose) printf("ThreadsJobControl running on %i cores.\n", ntasks);
    if(rank && verbose > 2) printf("ThreadsJobControl running on core %i.\n", rank);
    stillRunning.resize(ntasks+1);
}

bool ThreadsJobControl::_isRunning(int wid) {
    auto it = cthreads.find(wid);
    if(it == cthreads.end()) return false;
    if(stillRunning[it->first]) return true;
    pthread_join(it->second, nullptr);
    cthreads.erase(it);
    return true;
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
    int ret = std::system(a.s.c_str());
    if(ret) exit(99);
    stillRunning[a.r] = 0;
    return nullptr;
}

int ThreadsJobControl::_allocWorker() {
    while(checkJobs() == ntasks) usleep(10000); // wait for a slot

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
