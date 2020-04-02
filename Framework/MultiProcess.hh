/// \file MultiProcess.hh Parallel-processing pipeline manager
// -- Michael P. Mendenhall, 2018

#ifndef MultiProcess_HH
#define MultiProcess_HH

#include "ConfigProcess.hh"
#include "JobQueue.hh"

/// Parallel processing pipleline manager
class MultiProcess: public ConfigProcess, protected JobQueue {
public:
    /// Destructor
    virtual ~MultiProcess() { for(auto j: jpool) delete j; }

    /// start receiving a series of data frames
    void start_data(DataFrame& F) override;
    /// process next data frame in series
    void receive(DataFrame& F, FrameSource&) override;
    /// end series of data frames
    void end_data(DataFrame& F) override;
    /// Frame returned from child process
    void finished(DataFrame& F, FrameSink&) override;

protected:
    /// extra configuration after loading children
    void postconfig(const Setting& S) override;
    /// push along jobs pipeline (handled from main thread)
    void run_pipeline();

    /// wait until all queues are empty
    void flush() override;

    /// job queue wrapper for data frame handling
    class MPJob: public Job {
    public:
        /// Constructor
        explicit MPJob(MultiProcess* P): MP(P) { }
        /// perform processing step
        void run() override;
        MultiProcess* MP;           ///< MultiProcessor
        FrameSource* FS = nullptr;  ///< Frame return origin
        DataFrame* F = nullptr;     ///< job data
    };
    vector<MPJob*> jpool;           ///< re-usable allocated jobs

    vector<MPJob*> jdone;           ///< completed steps awaiting next process
    vector<MPJob*> getDone(bool wait = false);    ///< get (and clear) current jdone
    std::mutex jdoneLock;           ///< lock on job queues
    std::condition_variable jdonev; ///< wait for completed jobs

    std::mutex jsLock;              ///< lock on jsteps
    map<DataFrame*, MPJob*> jsteps; ///< jobs associated with frames
};

#endif
