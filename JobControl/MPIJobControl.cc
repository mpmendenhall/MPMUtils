/// @file MPIJobControl.cc
// -- Michael P. Mendenhall, LLNL 2019

#ifdef WITH_MPI
#include <mpi.h>
#include "MPIJobControl.hh"
#include <iostream> // for std::cout

MPIJobControl::~MPIJobControl() {
    // Send ending message to close worker process
    JobSpec JS0;
    for(auto r: availableRanks) {
        dataDest = r;
        send(JS0);
    }

    if(verbose > 1) printf(availableRanks.size()? "Controller [%i] closing.\n" : "Worker [%i] closing.\n", mpirank);
}

bool MPIJobControl::_isRunning(int wid) {
    // check for tag '1' message
    int flag = 0;
    int i = MPI_Iprobe(wid, 1, MPI_COMM_WORLD, &flag, MPI_STATUS_IGNORE);
    if(!flag) return true;

    MPI_Recv(&i, 1, MPI_INT, wid, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    availableRanks.insert(wid);
    return false;
}

int MPIJobControl::_allocWorker() {
    while(!availableRanks.size()) {
        if((int)checkJobs().size() < ntasks) break;
        usleep(10000);
    }

    int wid = *availableRanks.begin();
    availableRanks.erase(wid);
    return wid;
}

///////////////////////
///////////////////////

void MPIJobWorker::signalDone() {
    // send with tag '1' to signal done
    int i = 1;
    MPI_Send(&i, 1, MPI_INT, dataDest, 1, MPI_COMM_WORLD);
}

#endif
