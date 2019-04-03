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
    /// Constructor
    MPIJobControl() { ntasks = mpisize - 1; }
    /// Destructor (signals to close remote jobs)
    ~MPIJobControl();

protected:

    /// Check if a job is running or completed
    bool _isRunning(int) override;
    /// Allocate an available thread, blocking if necessary
    int _allocWorker() override;
};

/// Distribute and collect jobs over MPI
class MPIJobWorker: public MPIBinaryIO, public MultiJobWorker {
public:
    /// signal that job is done; ready for close-out comms
    void signalDone() override;
};

#endif
#endif
