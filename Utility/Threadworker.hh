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
#include <set>
using std::set;
#include <vector>
using std::vector;

class ThreadManager;

/// Utility base class for launching worker thread
class Threadworker {
public:
    /// Constructor
    explicit Threadworker(int i = 0, ThreadManager* m = nullptr);
    /// Destructor
    virtual ~Threadworker();

    /// launch worker thread (error if already launched)
    void launch_mythread();
    /// pause thread (blocks until threadjob() reciprocates)
    void pause();
    /// re-start paused thread (non-blocking)
    void unpause();
    /// set STOP_REQUESTED and notify (but do not wait for join)
    void request_stop();
    /// request and wait for completion of worker thread (error if not launched)
    void finish_mythread();
    /// force-kill a thread that refuses to finish
    void kill_mythread(double timeout_s = 0.01);
    /// run threadjob() in this thread; return when done
    void run_here();

    /// worker current status
    enum runstatus_t {
        IDLE            = 0,    ///< not currently running
        RUNNING         = 100,  ///< running started in separate thread
        RUNLOCAL        = 101,  ///< blocking run started in same thread
        PAUSE_REQUESTED = 200,  ///< requested pause
        PAUSED          = 201,  ///< in paused state
        STOP_REQUESTED  = 300,  ///< requested to finish running
        INDETERMINATE   = 400   ///< unknown/failed state
    };

    /// get launch status
    runstatus_t checkRunning() const { return runstat; }

    int worker_id;              ///< assignable identification number
    ThreadManager* myManager;   ///< link back to manager
    int verbose = 0;            ///< debugging verbosity level
    static int thread_id();     ///< worker_id that launched current thread

protected:
    /// task to be run in thread; example waiting for halt condition. Override me!
    virtual void threadjob();

    /// check for and respond to pause request
    void check_pause();

    /// pthreads function for launching processing loop
    static void* run_Threadworker_thread(void* p);

    pthread_t mythread;                 ///< identifier for this object's thread
    runstatus_t runstat = IDLE;         ///< current running status
    mutex inputMut;                     ///< mutex on input operations
    std::condition_variable inputReady; ///< input conditions change notifier
    mutex pauseMut;                     ///< mutex on pause operations
    std::condition_variable pauseReady; ///< pause conditions change notifier
};

/// Manage multiple worker threads
class ThreadManager: public Threadworker {
public:
    /// Destructor
    ~ThreadManager();

    /// add thread; optional automatic worker_id assignemnt
    void add_thread(Threadworker* t, bool autoid = true);

    /// notify (from thread) that it is completed and ready for pthread_join
    void notify_thread_completed(Threadworker* t);

    /// wait for all threads to complete
    void await_threads_completion();
    /// launch all threads, await_threads_completion()
    void threadjob() override;

protected:
    /// Callback on thread completion --- needs to perform approppriate memory management
    virtual void on_thread_completed(Threadworker* t) { if(t != this) delete t; }
    /// remove thread from mythreads
    void remove_thread(Threadworker* t);
    /// remove threads in pendingDone
    void purge_pending();

    set<Threadworker*> mythreads;       ///< managed threads
    vector<Threadworker*> pendingDone;  ///< threads reported back done, pending closeout processing
    int nrunning = 0;                   ///< counter on active running threads
};

#endif
