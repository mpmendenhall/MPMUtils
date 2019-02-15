/// \file MultiJobControl.hh Generic interface for distributing/receiving binary data/jobs
// Michael P. Mendenhall, LLNL 2019

#ifndef MULTIJOBCONTROL_HH
#define MULTIJOBCONTROL_HH

#include "BinaryIO.hh"
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

    template<class T>
    void pushState(const string& n, const T& d) {
        auto& p = stateData[n];
        if(p.alive) delete (T*)p.alive;
        p.alive = (void*)(new T(d));
        if(!persistent) p.dead = KeyData(d);
    }
    template<class T>
    void getState(const string& n, T& d) {
        auto it = stateData.find(n);
        assert(it != stateData.end());
        assert(bool(it->second.alive) == persistent);
        if(it->second.alive) d = *(T*)it->second.alive;
        else it->second.dead.Get(d);
    }

protected:

    int ntasks = 0;         ///< total number of job slots available
    int rank = 0;           ///< identifier for this job process
    int parentRank = 0;     ///< ``parent'' job number to return results
    bool persistent = true; ///< whether child processes are persistent or hne-shot
    bool runLocal = true;   ///< whether to run one portion of job locally on controller

    /// Check if a job is running or completed
    virtual bool _isRunning(int) = 0;
    /// Allocate an available worker; possibly blocking if necessary
    virtual int _allocWorker() = 0;

    /// Check if a job is running; perform end-of-job actions if completed.
    bool isRunning(int wid);
    /// Check status for all running jobs, performing post-return jobs as needed; return number of still-running jobs
    vector<int> checkJobs();

    map<int,JobSpec> jobs;          ///< active jobs by worker ID

    struct sData {
        void* alive = nullptr;      ///< functioning object
        KeyData dead;               ///< serialized data
    };
    map<string, sData> stateData;   ///< saved state information
};

#endif
