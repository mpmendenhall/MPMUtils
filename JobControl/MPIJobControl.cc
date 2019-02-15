/// \file MPIJobControl.cc
// Michael P. Mendenhall, LLNL 2019

#include "MPIJobControl.hh"

#ifdef WITH_MPI

void MPIJobControl::init(int argc, char **argv) {
    MPIBinaryIO::init(argc, argv);

    rank = mpirank;
    ntasks = mpisize - 1;
    parentRank = 0; //ntasks? 0 : (rank/coresPerNode)*coresPerNode;
    //if(rank > 1 && parentRank == 0) parentRank = 1;

    if(verbose) {
        std::cout << "Rank " << rank << " task of " << ntasks << " available on " << hostname;
        std::cout << " (" << coresPerNode << " cores) starting run.\n";
        std::cout << "\tParent: " << parentRank << "; children: <";
        for(auto r: availableRanks) std::cout << " " << r;
        std::cout << " >\n";
    }
}

void MPIJobControl::finish() {
    // Send ending message to close worker process
    JobSpec JS0;
    for(auto r: availableRanks) {
        dataDest = r;
        send(JS0);
    }

    if(verbose > 1) printf(availableRanks.size()? "Controller [%i] closing.\n" : "Worker [%i] closing.\n", rank);
    MPI_Finalize();
}

void MPIJobControl::signalDone() {
    // send with tag '1' to signal done
    int i = 1;
    MPI_Send(&i, 1, MPI_INT, dataDest, 1, MPI_COMM_WORLD);
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

#endif
