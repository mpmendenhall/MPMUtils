/// @file MultiJobControl.hh Generic interface for distributing/receiving binary data/jobs
// -- Michael P. Mendenhall, LLNL 2023

/*

CN = Controller Node
WN = (remote) Worker Node

JobWorker: instantiated on WN to run job according to supplied instructions
JobComm:   runs on CN, with protocol for communicating with JobWorker



CN: send JobSpec JS to WN, encoding JobWorker class wclass
    WN: Receives JS; loads appropriate worker W for JS.wclass;
        calls JobWorker::run(...) to load further start-of-run information
CN: Use JS.C->startJob(CN) to send additional configuration information to JobWorker on WN
    WN: finishes receiving additional configuration info; runs job
    WN: calls signalDone() when W is complete; sends output results to CN
CN: polls isRunning(...) until a job has completed; runs JS->C->endJob(CN) to receive output

*/

#ifndef MULTIJOBCONTROL_HH
#define MULTIJOBCONTROL_HH

#include "ObjectFactory.hh"
#include "BinaryIO.hh"
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



/// Base class defining communications protocol (over supplied channel) between controller and worker.
/// Runs on controller node to send start-of-job commands and process end-of-job results for each job instance.
class JobComm {
public:
    /// start-of-job communication (send instruction details specific to job type)
    virtual void startJob(BinaryIO&) = 0;
    /// end-of-job communication (retrieve results specific to job type)
    virtual void endJob(BinaryIO&) = 0;

    /// Helper function to create subdivided jobs list, referencing this communicator
    void splitJobs(vector<JobSpec>& vJS, size_t nSplit, size_t nItms, size_t wclass, int uid=0);
};



/// Base class for a worker job; subclass and REGISTER_FACTORYOBJECT(myClass, JobWorker) in your code
class JobWorker {
public:
    /// polymorphic destructor
    virtual ~JobWorker() { }

    /// run specified job, talking to JobComm::startJob and endJob through B
    virtual void run(const JobSpec& J, BinaryIO& B);
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

    map<int,JobSpec> jobs;      ///< active jobs by worker ID
};


/// Worker node able to run different job types
class MultiJobWorker: virtual public BinaryIO {
public:
    /// Destructor
    ~MultiJobWorker() { for(const auto& kv: workers) delete kv.second; }
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
