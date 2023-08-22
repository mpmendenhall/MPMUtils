/// \file KTAccumJob.hh KeyTable-based accumulate protocol communicator
// -- Michael P. Mendenhall, LLNL 2019

#ifndef KTACCUMJOB_HH
#define KTACCUMJOB_HH

#include "MultiJobControl.hh"
#include "KeyTable.hh"
#include <TH1.h>

/// KeyTable-based accumulate protocol communicator
class KTAccumJobComm: public JobComm {
public:
    /// Destructor
    virtual ~KTAccumJobComm() { for(auto p: objs) delete p; }

    KeyTable kt;    ///< associated KeyTable

    /// start-of-job communication (send instruction details)
    void startJob(BinaryIO& B) override { B.send(kt); }
    /// end-of-job communication (get returnCombined() results)
    void endJob(BinaryIO& B) override;

    /// collect accumulated objects back into kt
    virtual void gather();

    /// get correct worker class ID
    virtual size_t workerType() const = 0;

    /// launch accumulation jobs
    void launchAccumulate(int uid = 0);

protected:
    vector<string> combos;  ///< accumulation object names
    vector<TH1*> objs;      ///< accumulation TH1's
};

/// Base job working with KTAccumJobComm
class KTAccumJob: public JobWorker {
public:
    /// run job
    void run(const JobSpec& J, BinaryIO& B) override;

    KeyTable kt;    ///< received KeyTable data

protected:
    /// subclass me with calculation on kt, J!
    virtual void runAccum() { printf("KTAccumJob does nothing for "); JS.display(); }
    /// Return 'combine' entries from a KeyTable
    void returnCombined(BinaryIO& B);

    JobSpec JS;     ///< current job info
};


#endif
