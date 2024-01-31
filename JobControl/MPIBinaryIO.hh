/// @file MPIBinaryIO.hh Binary I/O over MPI channel

#ifndef MPIBINARYIO_HH
#define MPIBINARYIO_HH

#include "BinaryIO.hh"
#include <set>
using std::set;

/// Binary I/O over MPI, with static MPI instance info
class MPIBinaryIO: virtual public BinaryIO {
public:
    /// initialize with MPI information
    static void init(int argc, char **argv);
    /// close out MPI
    static void uninit();
    /// display MPI info to stdout
    static void display();

    static int mpisize;             ///< number of MPI ranks available
    static int mpirank;             ///< this job's number
    static char* hostname;          ///< hostname for this machine
    static int coresPerNode;        ///< number of cores on this MPI node
    static set<int> availableRanks; ///< communication ranks available

protected:
    /// blocking data send
    void _send(const void* vptr, size_t size) override;
    /// blocking data receive
    void read(void* vptr, size_t size) override;

    vector<char> rbuff; ///< read buffer
    size_t rpt = 0;     ///< current read buffer read point
    int dataSrc = 0;        ///< source identifier for data received
};

#endif
