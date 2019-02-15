/// \file MPIBinaryIO.hh Binary I/O over MPI channel

#ifdef WITH_MPI
#ifndef MPIBINARYIO_HH
#define MPIBINARYIO_HH

#include <mpi.h>
#include "BinaryIO.hh"
#include <set>
using std::set;

/// Binary I/O over MPI
class MPIBinaryIO: virtual public BinaryIO {
public:
    /// initialize with MPI information
    void init(int argc, char **argv);

protected:
    /// blocking data send
    void _send(void* vptr, int size) override;
    /// blocking data receive
    void _receive(void* vptr, int size) override;

    int mpisize = 0;                        ///< number of MPI ranks available
    int mpirank = 0;                        ///< this job's number
    char hostname[MPI_MAX_PROCESSOR_NAME];  ///< hostname for this machine
    int coresPerNode = 0;                   ///< number of cores on this MPI node
    set<int> availableRanks;                ///< communication ranks available
};

#endif
#endif
