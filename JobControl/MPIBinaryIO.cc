/// \file MPIBinaryIO.cc
// Michael P. Mendenhall, LLNL 2019

#ifdef WITH_MPI
#include "MPIBinaryIO.hh"
#include <cstring> // for std::memcpy
#include <iostream> // for std::cout

void MPIBinaryIO::_send(void* vptr, int size) {
    if(!size) return;
    MPI_Send(vptr, size, MPI_UNSIGNED_CHAR, dataDest, 2, MPI_COMM_WORLD);
}

void MPIBinaryIO::_receive(void* vptr, int size) {
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

    std::memcpy(vptr, rbuff.data()+rpt, size);
    rpt += size;
}

void MPIBinaryIO::init(int argc, char **argv) {
    int status = MPI_Init(&argc, &argv);
    if(status != MPI_SUCCESS) {
        std::cout << "MPI Init Error.\n";
        MPI_Abort(MPI_COMM_WORLD,status);
    }

    // Get information about task.
    MPI_Comm_rank(MPI_COMM_WORLD, &mpirank);
    MPI_Comm_size(MPI_COMM_WORLD, &mpisize);
    int length = 0;
    MPI_Get_processor_name(hostname, &length);
    coresPerNode = atol(getenv("SLURM_CPUS_ON_NODE"));

    if(mpisize <= coresPerNode) { // local single-level distribution

        if(!mpirank) for(int i = 1; i < mpisize; i++) availableRanks.insert(i);

    } else { // multi-level distribution

        if(!mpirank) { // top level

            int numControllers = mpisize/coresPerNode;
            for(int i = 0; i < numControllers; i++) {
                int r = i*coresPerNode;
                availableRanks.insert(r? r : 1);
            }

        } else if(mpirank == 1 || mpirank % coresPerNode == 0) { // controller nodes

            int rankStart = mpirank + 1;
            int rankEnd = ((mpirank/coresPerNode)+1)*coresPerNode;
            for(int i=rankStart; i<rankEnd; i++) availableRanks.insert(i);
        }
    }
}

#endif
