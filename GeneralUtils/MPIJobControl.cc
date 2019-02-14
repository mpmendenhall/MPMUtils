/// \file MPIJobControl.cc
// Michael P. Mendenhall, LLNL 2019

#include "MPIJobControl.hh"

void MPIJobControl::_send(void* vptr, int size) {
    MPI_Send(vptr, size, MPI_UNSIGNED_CHAR, dataDest, 0, MPI_COMM_WORLD);
}

void MPIJobControl::_receive(void* vptr, int size) {
    MPI_Recv(vptr, size, MPI_UNSIGNED_CHAR, dataSrc, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}

void MPIJobControl::init(int argc, char **argv) {
    int status = MPI_Init(&argc, &argv);
    if(status != MPI_SUCCESS) {
        std::cout << "MPI Init Error.\n";
        MPI_Abort(MPI_COMM_WORLD,status);
    }

    // Get information about task.
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &ntasks);
    int length = 0;
    MPI_Get_processor_name(hostname, &length);
    coresPerNode = atol(getenv("SLURM_CPUS_ON_NODE"));

    if(ntasks <= coresPerNode) { // local single-level distribution

        if(!rank) for(int i = 1; i < ntasks; i++) childRanks.push_back(i);

    } else { // multi-level distribution

        if(!rank) { // top level

            int numControllers = ntasks/coresPerNode;
            for(int i = 0; i < numControllers; i++) {
                int r = i*coresPerNode;
                childRanks.push_back(r? r : 1);
            }

        } else if(rank == 1 || rank % coresPerNode == 0) { // controller nodes

            int rankStart = rank + 1;
            int rankEnd = ((rank/coresPerNode)+1)*coresPerNode;
            for(int i=rankStart; i<rankEnd; i++) childRanks.push_back(i);
        }

        parentRank = childRanks.size()? 0 : (rank/coresPerNode)*coresPerNode;
        if(rank > 1 && parentRank == 0) parentRank = 1;
    }

    if(verbose) {
        std::cout << "Rank " << rank << " task of " << ntasks << " available on " << hostname;
        std::cout << " (" << coresPerNode << " cores) starting run.\n";
        std::cout << "\tParent: " << parentRank << "; children: <";
        for(auto r: childRanks) std::cout << " " << r;
        std::cout << " >\n";
    }
}

void MPIJobControl::finish() {
    // Send ending message to close worker process
    JobSpec JS0;
    for(auto r: childRanks) {
        dataDest = r;
        send(JS0);
    }

    if(verbose > 1) printf(childRanks.size()? "Controller [%i] closing.\n" : "Worker [%i] closing.\n", rank);
    MPI_Finalize();
}


bool MPIJobControl::_isRunning(int wid) {
    assert(false);
}

int MPIJobControl::_allocWorker() {
    while(checkJobs() == ntasks) usleep(10000); // wait for a slot

    assert(false);
    for(int i=0; i<ntasks; i++) {

        return i;
    }

    exit(1); // should have a slot available???
}

