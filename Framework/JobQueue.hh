/// \file JobQueue.hh Parallel-processing pipeline management
// Michael P. Mendenhall, 2018

#ifndef JOBQUEUE_HH
#define JOBQUEUE_HH

#include <vector>
using std::vector;
#include <deque>
using std::deque;
#include <map>
using std::map;
#include <unistd.h>
#include <condition_variable>
#include <stdio.h>
#include <boost/core/noncopyable.hpp>

/// Parallel-processing pipeline management
class JobQueue: private boost::noncopyable {
public:
    /// Destructor
    virtual ~JobQueue() { shutdown(); }

    /// Base class defining a job to run
    class Job {
    public:
        /// Destructor
        virtual ~Job() { }
        /// Constructor
        Job(int n = 0): qn(n) { }
        /// run job (subclass me!)
        virtual void run() { printf("Hello, I'm a sleepy job on queue %i!\n", qn); usleep(1000000); }
        int qn; ///< queue category identifier, for multiple independent queues
    };

    /// configure queue settings
    void setQueue(int qn, size_t max_workers, size_t backlog = 10000);
    /// add a job, waiting as needed until queue is down to 'backlog' entries
    void add(Job* J);
    /// launch controller thread to start job processing on specified number of workers
    void launch(size_t nw);
    /// wait until all queues are empty
    virtual void flush();
    /// flush and close worker/controller threads
    void shutdown();
    /// display current queue status
    void display();

    int verbose = 0;    ///< debugging verbosity

protected:

    /// thread waiting for single jobs to run
    class jthread {
    public:
        /// Constructor, spins up thread
        jthread(JobQueue& jq);
        /// Destructor, waits to close thread
        ~jthread();

        /// Launch specified job
        void launch(Job* j);

        pthread_t t = 0;        ///< the thread
        Job* jToRun = nullptr;  ///< next job to run
        std::mutex jlock;       ///< job ready lock
        std::condition_variable jready; ///< input ready wait condition
        JobQueue& JQ;           ///< parent controller

    protected:
        /// worker thread, waits for and runs jobs
        static void* jqworkthread(void* vJT);
        /// worker operation
        void doWork();
        /// thread halt job marker
        static Job haltThread;
    };

    /// controller thread, dispatches to available workers
    static void* jqcontrolthread(void* jJQ);
    /// controller operation
    void runController();
    pthread_t cThrd = 0;    ///< thread running controller

    size_t nworkers = 0;        ///< maximum allowed worker threads
    vector<jthread*> j_idle;    ///< idle threads waiting for jobs
    std::condition_variable wready; ///< wait for idle worker
    std::mutex idleLock;        ///< lock on idle pool
    jthread* getIdle();         ///< return next available idle worker

    /// queue for a particular kind of job
    struct jqueue {
        size_t max_workers = 1000;      ///< max. parallel jobs (1 for strictly serial)
        size_t n_workers = 0;           ///< current number of running processes
        size_t backlog = 10000;         ///< maximum backlog before holding up new job submissions
        deque<Job*> js;                 ///< FIFO queue of jobs
    };
    jqueue* chooseNext();               ///< select next queue to run job from (nullptr if none possible)
    jqueue* nextQ = nullptr;            ///< most-recently-selected next queue
    map<int,jqueue> jqs;                ///< queue categories
    std::mutex jqsLock;                 ///< lock on job queues
    size_t nwaiting = 0;                ///< total number of jobs waiting to be run
    std::condition_variable v_jnew;     ///< wait for new available job
    std::condition_variable v_jdone;    ///< wait for new available job

    bool halt = true;                   ///< signal for threads to halt
};

#endif
