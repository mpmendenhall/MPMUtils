/// \file MPIJobControl.hh MultiJobControl communicating via MPI
// Michael P. Mendenhall, LLNL 2019

#ifdef WITH_MPI
#ifndef MPIJOBCONTROL_HH
#define MPIJOBCONTROL_HH

#include <mpi.h>
#include "MultiJobControl.hh"
#include <set>
using std::set;

/// Distribute and collect jobs over MPI
class MPIJobControl: public MultiJobControl {
public:
    /// initialize with MPI information
    void init(int argc, char **argv) override;
    /// end-of-run completion
    void finish() override;

    /// signal that job is done; ready for close-out comms
    void signalDone() override;

    char hostname[MPI_MAX_PROCESSOR_NAME];  ///< hostname for this machine

protected:
    set<int> availableRanks;    ///< ranks available to take a new job

    /// blocking data send
    void _send(void* vptr, int size) override;
    /// blocking data receive
    void _receive(void* vptr, int size) override;

    vector<char> rbuff; /// read buffer
    size_t rpt = 0;     /// position in read buffer

    /// Check if a job is running or completed
    bool _isRunning(int) override;
    /// Allocate an available thread, blocking if necessary
    int _allocWorker() override;
};

#endif
#endif
