/// \file MultiJobControl.hh Generic interface for distributing/receiving binary data/jobs
// Michael P. Mendenhall, LLNL 2019

#ifndef MULTIJOBCONTROL_HH
#define MULTIJOBCONTROL_HH

#include "BinaryIO.hh"
#include "ObjectFactory.hh"
#include "KeyTable.hh"
#include <unistd.h>
#include <stdexcept>

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

/// Base class for communicating with a worker
class JobComm {
public:
    /// start-of-job communication (send instruction details)
    virtual void startJob(BinaryIO&) { }
    /// end-of-job communication (retrieve results)
    virtual void endJob(BinaryIO&) { }

    /// Helper function to create subdivided jobs list, referencing this communicator
    void splitJobs(vector<JobSpec>& vJS, size_t nSplit, size_t nItms, size_t wclass, int uid=0);
};


/// Base class for a worker job; subclass and REGISTER_FACTORYOBJECT(myClass) in your code
class JobWorker: public FactoryObject {
public:
    /// run specified job, talking to JobComm::startJob and endJob through B
    virtual void run(JobSpec J, BinaryIO& B);
};

/// Distribute and collect jobs
class MultiJobControl: virtual public BinaryIO {
public:

    /// run as worker, processing and returning jobs
    void runWorker();

    /// Submission of job for processing; updates and returns JS.wid with assigned worker number. Possibly blocking depending on jobs back-end.
    int submitJob(JobSpec& JS);

    /// Blocking wait for listed jobs to have finished
    void waitFor(const vector<int>& v);
    /// Blocking wait for all jobs to complete
    void waitComplete();

    /// start-of-process initialization
    virtual void init(int argc, char **argv) = 0;
    /// end-of-run completion
    virtual void finish() = 0;
    /// recommended number of parallel tasks
    virtual size_t nChunk() const { return ntasks; }
    /// check if is top-level controller
    bool isController() const { return !rank; }
    /// signal that job is done; ready for close-out comms
    virtual void signalDone() { }

    static MultiJobControl* JC; ///< singleton instance for job control type
    int verbose = 0;            ///< debugging verbosity level

    /// check if state data available (and make available if possible) for hash
    virtual bool checkState(size_t h);
    /// clear state data for hash
    virtual void clearState(size_t h);

    /// push state data for identifier hash
    template<class T>
    void pushState(size_t h, const T& d) {
        stateData.emplace(h,d);
        persistState(h);
    }

    /// load state data for identifier hash
    template<class T>
    void getState(size_t h, T& d) {
        if(!checkState(h)) throw std::range_error("State data unavailable");
        stateData.at(h).Get(d);
    }

    string stateDir = "";   ///< non-empty to specify directory for state data storage

protected:

    int ntasks = 0;         ///< total number of job slots available
    int rank = 0;           ///< identifier for this job process
    int parentRank = 0;     ///< ``parent'' job number to return results
    bool persistent = true; ///< whether child processes are persistent or one-shot
    bool runLocal = true;   ///< whether to run one portion of job locally on controller

    /// name for state data file
    virtual string sdataFile(size_t h) const;
    /// persistently save state data for hash
    virtual void persistState(size_t h);

    /// Check if a job is running or completed
    virtual bool _isRunning(int) = 0;
    /// Allocate an available worker; possibly blocking if necessary
    virtual int _allocWorker() = 0;

    /// Check if a job is running; perform end-of-job actions if completed.
    bool isRunning(int wid);
    /// Check status for all running jobs, performing post-return jobs as needed; return number of still-running jobs
    vector<int> checkJobs();

    map<int,JobSpec> jobs;          ///< active jobs by worker ID
    map<size_t, KeyData> stateData; ///< memory-resident saved state information by hash
    map<size_t, size_t> lastReq;    ///< when each piece of stored data was last requested
    size_t nReq = 0;                ///< number of times stored data has been requested
};

#endif
