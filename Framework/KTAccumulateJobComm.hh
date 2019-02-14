/// \file KTAccumulateJobComm.hh KeyTable-based accumulate protocol communicator
// Michael P. Mendenhall, LLNL 2019

#ifndef KTACCUMULATEJOBCOMM_HH
#define KTACCUMULATEJOBCOMM_HH

#include "MultiJobControl.hh"
#include "KeyTable.hh"
#include <TH1.h>

/// KeyTable-based accumulate protocol communicator
class KTAccumulateJobComm: public JobComm {
public:
    /// Destructor
    virtual ~KTAccumulateJobComm() { for(auto p: objs) delete p; }

    KeyTable kt;            ///< associated KeyTable

    /// start-of-job communication (send instruction details)
    void startJob(BinaryIO& B) override { B.send(kt); }
    /// end-of-job communication (get returnCombined() results)
    void endJob(BinaryIO& B) override;

    /// collect accumulated objects back into kt
    void gather();

    /// launch accumulation jobs
    void launchAccumulate(size_t wid, int uid = 0);

protected:
    vector<string> combos;  ///< accumulation object names
    vector<TH1*> objs;      ///< accumulation TH1's
};

/// Base job working with KTAccumulateJobComm
class KTAccumJob: public JobWorker {
public:
    /// run job
    void run(JobSpec J, BinaryIO& B) override;

protected:
    /// subclass me with calculation on kt, J!
    virtual void run(JobSpec J) { printf("KTAccumJob does nothing for "); J.display(); }
    /// Return 'combine' entries from a KeyTable
    void returnCombined(BinaryIO& B);

    KeyTable kt; ///< received KeyTable data
};


#endif
