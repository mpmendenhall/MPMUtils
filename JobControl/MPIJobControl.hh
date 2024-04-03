/// @file MPIJobControl.hh MultiJobControl communicating via MPI
// -- Michael P. Mendenhall, LLNL 2019

#ifndef MPIJOBCONTROL_HH
#define MPIJOBCONTROL_HH

#include "MPIBinaryIO.hh"
#include "MultiJobControl.hh"

#ifdef WITH_MPI

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
    /// Constructor
    MPIJobWorker() { wid = MPIBinaryIO::mpirank; }
    /// signal that job is done; ready for close-out comms
    void signalDone() override;
};

#else

/// Distribute and collect jobs over MPI
class MPIJobControl: public MPIBinaryIO, public MultiJobControl {
public:
    /// Constructor
    MPIJobControl() { throw std::logic_error("MPI support disabled!"); }
protected:
    /// Check if a job is running or completed
    bool _isRunning(int) override { return false; }
    /// Allocate an available thread, blocking if necessary
    int _allocWorker() override { return 0; }
};

/// Distribute and collect jobs over MPI
class MPIJobWorker: public MPIBinaryIO, public MultiJobWorker {
public:
    /// Constructor
    MPIJobWorker() { throw std::logic_error("MPI support disabled!"); }
};

#endif
#endif
