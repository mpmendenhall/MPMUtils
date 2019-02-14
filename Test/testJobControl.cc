/// \file testJobControl.cc Test of job control classes

#include "ThreadsJobControl.hh"
#include "KTAccumJob.hh"
#include <TH1F.h>

/// Test job class interfacing with KeyData
class MyAccumJob: public KTAccumJob {
protected:
    /// operate on kt data to produce accumulables
    void run(JobSpec S) {
        auto f = kt.GetROOT<TH1>("v");
        assert(f);
        for(int i=0; i<10; i++) f->Fill(i+0.5);
        printf("Integral %g in ", f->Integral());
        S.display();
        kt.Set("v", *f);
    }
};

REGISTER_FACTORYOBJECT(MyAccumJob)

/// main() for executable
int main(int argc, char **argv) {

    MultiJobControl::JC = new ThreadsJobControl();
    MultiJobControl::JC->verbose = 5;
    MultiJobControl::JC->init(argc, argv);

    if(MultiJobControl::JC->rank == 0) {

        KTAccumJobComm KTC;
        auto foo = new TH1F("foo","bar",20,0,10);
        KTC.kt.Set("v", *foo);
        KTC.kt.Set("Combine","v");

        for(int i=0; i<10; i++) {
            JobSpec JS;
            JS.uid = i;
            if(i%3) JS.wclass = typehash<JobWorker>();
            else {
                JS.wclass = typehash<MyAccumJob>();
                JS.C = &KTC;
            }
            MultiJobControl::JC->submitJob(JS);
        }
        printf("\n\nAll submitted!\n\n");

        MultiJobControl::JC->waitComplete();

        printf("\n\nAll done!\n");

        KTC.gather();
        auto f = KTC.kt.GetROOT<TH1>("v");
        assert(f);
        for(int i=1; i<=f->GetNbinsX(); i++) printf("\t%i\t%g\n", i, f->GetBinContent(i));

    } else MultiJobControl::JC->runWorker();

    MultiJobControl::JC->finish();

    return EXIT_SUCCESS;
}
