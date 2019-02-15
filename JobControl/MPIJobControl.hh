/// \file MPIJobControl.hh MultiJobControl communicating via MPI
// Michael P. Mendenhall, LLNL 2019

#ifdef WITH_MPI
#ifndef MPIJOBCONTROL_HH
#define MPIJOBCONTROL_HH

#include "MPIBinaryIO.hh"
#include "MultiJobControl.hh"

/// Distribute and collect jobs over MPI
class MPIJobControl: public MPIBinaryIO, public MultiJobControl {
public:
    /// initialize with MPI information
    void init(int argc, char **argv) override;
    /// end-of-run completion
    void finish() override;

    /// signal that job is done; ready for close-out comms
    void signalDone() override;

protected:

    /// Check if a job is running or completed
    bool _isRunning(int) override;
    /// Allocate an available thread, blocking if necessary
    int _allocWorker() override;
};

#endif
#endif
