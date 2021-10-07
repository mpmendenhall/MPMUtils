/// \file Threadworker.hh Utility base class for launching worker thread
// Michael P. Mendenhall, LLNL 2021

#ifndef THREADWORKER_HH
#define THREADWORKER_HH

#include <pthread.h>
#include <sched.h>
#include <stdexcept>
#include <condition_variable>
#include <mutex>
using std::mutex;
using std::lock_guard;
using std::unique_lock;

/// Utility base class for launching worker thread
class Threadworker {
public:
    /// Destructor
    virtual ~Threadworker() { if(is_launched) finish_mythread(); }

    /// task to be run in thread; example waiting for halt condition. Override me!
    virtual void threadjob();

    /// launch worker thread (error if already launched)
    void launch_mythread();
    /// request and wait for completion of worker thread (error if not launched)
    void finish_mythread();
    /// get launch status
    bool checkRunning() const { return is_launched; }

protected:
    /// pthreads function for launching processing loop
    static void* run_Threadworker_thread(void* p);

    bool all_done = false;      ///< flag to indicate when all input operations are complete
    pthread_t mythread;         ///< identifier for this object's thread
    bool is_launched = false;   ///< marker for whether thread is launched
    mutex inputMut;             ///< mutex on input operations
    std::condition_variable inputReady; ///< input conditions notifier
};

#endif
