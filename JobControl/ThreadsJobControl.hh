/// \file ThreadsJobControl.hh Threading-based job runner

#ifndef THREADSJOBCONTROL_HH
#define THREADSJOBCONTROL_HH

#include "DiskIOJobControl.hh"

/// Distribute and collect jobs via filesystem and batch queue
class ThreadsJobControl: public DiskIOJobControl {
public:
    /// initialize
    void init(int argc, char** argv) override;

protected:
    /// Check if a job is running or completed
    bool _isRunning(int) override;
    /// Allocate an available thread, blocking if necessary
    int _allocWorker() override;

    string exec_name;               ///< executable name
    map<int, pthread_t> cthreads;   ///< child threads
};

#endif
