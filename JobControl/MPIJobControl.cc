/// \file MPIJobControl.cc
// Michael P. Mendenhall, LLNL 2019

#include "MPIJobControl.hh"

#ifdef WITH_MPI

void MPIJobControl::_send(void* vptr, int size) {
    if(!size) return;
    MPI_Send(vptr, size, MPI_UNSIGNED_CHAR, dataDest, 2, MPI_COMM_WORLD);
}

void MPIJobControl::_receive(void* vptr, int size) {
    if(!size) return;

    if(rpt == rbuff.size()) { // need new data
        MPI_Status status;
        MPI_Probe(dataSrc, 2, MPI_COMM_WORLD, &status);
        int msize = 0;
        MPI_Get_count(&status, MPI_UNSIGNED_CHAR, &msize);
        rbuff.resize(msize);
        MPI_Recv(rbuff.data(), msize, MPI_UNSIGNED_CHAR, dataSrc, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        rpt = 0;
    }

    if(rpt + size > rbuff.size()) {
        printf("*** Error: unexpected MPI data boundary!");
        exit(88);
    }

    memcpy(vptr, rbuff.data()+rpt, size);
    rpt += size;
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

        if(!rank) for(int i = 1; i < ntasks; i++) availableRanks.insert(i);

    } else { // multi-level distribution

        if(!rank) { // top level

            int numControllers = ntasks/coresPerNode;
            for(int i = 0; i < numControllers; i++) {
                int r = i*coresPerNode;
                availableRanks.insert(r? r : 1);
            }

        } else if(rank == 1 || rank % coresPerNode == 0) { // controller nodes

            int rankStart = rank + 1;
            int rankEnd = ((rank/coresPerNode)+1)*coresPerNode;
            for(int i=rankStart; i<rankEnd; i++) availableRanks.insert(i);
        }

        parentRank = availableRanks.size()? 0 : (rank/coresPerNode)*coresPerNode;
        if(rank > 1 && parentRank == 0) parentRank = 1;
    }

    ntasks = availableRanks.size();

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
