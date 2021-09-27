/// \file MultiJobControl.hh Generic interface for distributing/receiving binary data/jobs
// -- Michael P. Mendenhall, LLNL 2019

/*

JobWorker: instantiated on remote node to run job according to supplied instructions
JobComm:   lives on control node with protocol for communicating with JobWorker; holds job-specific configuration info

CN = Controller Node
WN = Worker Node

CN: send JobSpec JS to WN, encoding JobWorker class wclass
CN: run JS.C->startJob(CN) to send additional configuration information
WN: Receives JS; loads appropriate worker W for JS.wclass
CN: Use JS->C->startJob(CN as comm channel) to send start-of-job instructions
WN: W->Run(JS, WN as comm channel); might await/receive config info from remote JS->C
WN: calls signalDone() when W is complete
CN: polls isRunning(...) until a job has completed; runs JS->C->endJob(CN) to receive output

*/

#ifndef MULTIJOBCONTROL_HH
#define MULTIJOBCONTROL_HH

#include "ObjectFactory.hh"
#include "KeyTable.hh"
#include <unistd.h>

class JobComm;

/// Basic job information
struct JobSpec {
    int uid = 0;            ///< unique identifier for this job (associated with persisted data)
    int wid = 0;            ///< worker ID (assigned by Job Control, e.g. an MPI rank)
    size_t wclass = 0;      ///< worker type identifier; 0 for stop job
    size_t N0 = 0;          ///< starting subdivision range
    size_t N1 = 0;          ///< ending range subdivision
    JobComm* C = nullptr;   ///< communicator for relaying job details and results

    void display() const;   ///< print summary to stdout
};



/// Base class defining communications protocol (over supplied channel) between controller and worker
class JobComm {
public:
    /// start-of-job communication (send instruction details)
    virtual void startJob(BinaryIO&) = 0;
    /// end-of-job communication (retrieve results)
    virtual void endJob(BinaryIO&) = 0;

    /// Helper function to create subdivided jobs list, referencing this communicator
    void splitJobs(vector<JobSpec>& vJS, size_t nSplit, size_t nItms, size_t wclass, int uid=0);
};



/// Base class for a worker job (with state storage utilities); subclass and REGISTER_FACTORYOBJECT(myClass, JobWorker) in your code
class JobWorker {
public:
    /// polymorphic destructor
    virtual ~JobWorker() { }

    /// run specified job, talking to JobComm::startJob and endJob through B
    virtual void run(const JobSpec& J, BinaryIO& B);

    /// check if state data available (and make available if possible) for hash
    virtual bool checkState(const string& h);
    /// clear state data for hash
    virtual void clearState(const string& h);

    /// push state data for identifier hash
    template<class T>
    void pushState(const string& h, const T& d) {
        stateData.emplace(h,d);
        persistState(h);
    }

    /// load state data for identifier hash
    template<class T>
    void getState(const string& h, T& d) {
        if(!checkState(h)) throw std::range_error("State data unavailable");
        stateData.at(h).Get(d);
    }

    static string stateDir;         ///< non-empty to specify directory for state data storage

protected:
    /// name for state data file
    virtual string sdataFile(const string& h) const;
    /// persistently save state data for hash
    virtual void persistState(const string& h);

    map<string, KeyData> stateData; ///< memory-resident saved state information by hash
    map<string, size_t> lastReq;    ///< when each piece of stored data was last requested
    size_t nReq = 0;                ///< number of times stored data has been requested
};




/// Controller node distributing jobs to matching MultiJobWorker; subclass with features for a particular channel.
class MultiJobControl: virtual public BinaryIO {
public:
    /// Submission of job for processing; updates and returns JS.wid with assigned worker number. Possibly blocking depending on jobs back-end.
    virtual int submitJob(JobSpec& JS);
    /// Blocking wait for listed jobs to have finished
    void waitFor(const vector<int>& v);
    /// Blocking wait for all jobs to complete
    void waitComplete();

    /// recommended number of parallel tasks
    virtual size_t nChunk() const { return ntasks; }

    static MultiJobControl* JC; ///< singleton instance for job control type
    int verbose = 0;            ///< debugging verbosity level

protected:

    int ntasks = 1;             ///< total number of job slots available

    /// Check if a job is running or completed
    virtual bool _isRunning(int) = 0;
    /// Allocate an available worker; possibly blocking if necessary
    virtual int _allocWorker() = 0;

    /// Check if a job is running; perform end-of-job actions if completed.
    bool isRunning(int wid);
    /// Check status for all running jobs, performing post-return jobs as needed; return number of still-running jobs
    vector<int> checkJobs();

    map<int,JobSpec> jobs;          ///< active jobs by worker ID
};


/// Worker node able to run different job types
class MultiJobWorker: virtual public BinaryIO {
public:
    /// Destructor
    ~MultiJobWorker() { for(auto& kv: workers) delete kv.second; }
    /// run as worker, processing and returning jobs
    void runWorkerJobs();

    /// signal that worker job is done, and ready for close-out comms
    virtual void signalDone() { }

    static MultiJobWorker* JW;          ///< singleton instance for job control type
    int verbose = 0;                    ///< debugging verbosity level

protected:

    /// Run a single job
    void runJob(JobSpec& JS);

    bool persistent = true;             ///< whether child processes are persistent or one-shot
    map<size_t, JobWorker*> workers;    ///< workers by class
};




//////////////////////////////////////
//////////////////////////////////////

/// Run a single job locally
class LocalJobControl: public DequeBIO, public MultiJobControl, public MultiJobWorker {
public:
    /// Submit job and run to completion.
    int submitJob(JobSpec& JS) override;
protected:
    /// Check if a job is running or completed
    bool _isRunning(int) override { return false; }
    /// Allocate an available thread, blocking if necessary
    int _allocWorker() override { return 0; }
};

#endif
