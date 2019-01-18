/// \file MPIJobControl.hh Helper utility for distributing data and jobs over MPI

#ifndef MPIJOBCONTROL_HH
#define MPIJOBCONTROL_HH

#include "KeyTable.hh"
#include <mpi.h>

/// Distribute and collect jobs over MPI
class MPIJobControl {
public:
    /// initialize with MPI information
    static void init(int argc, char **argv);
    /// run as intermediate controller node (automatic from init)
    static void runController();
    /// run as worker, processing and returning jobs
    static void runWorker();
    /// Distribute processing to child nodes; accumulate returned results (and return if not parent)
    static void accumulate(KeyTable& kt);
    /// Return accumulated results to parent
    static void returnAccumulate(KeyTable& kt);

    /// *** Define me somewhere! *** for base-level worker job
    static void workerJob(KeyTable& kt);

    static int ntasks;          ///< total number of MPI jobs available
    static int rank;            ///< this job's number
    static int coresPerNode;    ///< number of cores per node
    static int parentRank;      ///< ``parent'' job number to return results
    static vector<int> childRanks;  ///< ``child'' job id's for farming out jobs
    static int verbose;         ///< debugging verbosity level
    static char hostname[MPI_MAX_PROCESSOR_NAME];   ///< hostname for this machine
};



////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////



/// Send data with MPI
inline void mpiSend(int rank, void* vptr, int size) {
    MPI_Send(vptr, size, MPI_UNSIGNED_CHAR, rank, 0, MPI_COMM_WORLD);
}
/// Receive data with MPI
inline void mpiReceive(int rank, void* vptr, int size) {
    MPI_Recv(vptr, size, MPI_UNSIGNED_CHAR, rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}

/// generic MPI data send
template<typename T>
void mpiSend(int rank, const T& v) { mpiSend(rank, (void*)&v, sizeof(T)); }

/// generic MPI data receive
template<typename T>
T mpiReceive(int rank) {
    T v;
    mpiReceive(rank, (void*)&v, sizeof(T));
    return v;
}

/// vector MPI data send
template<typename T>
void mpiSend(int rank, const vector<T>& v) {
    mpiSend<int>(rank, v.size());
    for(auto& x: v) mpiSend<T>(rank, x);
}

/// vector MPI data send
template<typename T>
void mpiSendVec(int rank, const vector<T>& v) {
    mpiSend<int>(rank, v.size());
    for(auto& x: v) mpiSend<T>(rank, x);
}

/// vector MPI receive
template<typename T>
vector<T> mpiReceiveVec(int rank) {
    vector<T> v(mpiReceive<int>(rank));
    for(auto& x: v) x = mpiReceive<T>(rank);
    return v;
}

/// string MPI send
template<>
void mpiSend<string>(int rank, const string& s);

/// Receive string with MPI.
template<>
string mpiReceive<string>(int rank);

/// Send KeyTable with MPI
template<>
void mpiSend<KeyTable>(int rank, const KeyTable& kt);

/// Receive KeyTable with MPI
template<>
KeyTable mpiReceive<KeyTable>(int rank);

/// Send KeyData buffered object
template<>
void mpiSend(int rank, const KeyData& M);
/// Receive (and accept memory management of) KeyData
template<>
KeyData* mpiReceive<KeyData*>(int rank);

#endif
