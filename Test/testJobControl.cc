/// \file testJobControl.cc Test of job control classes

/*
export SLURM_CPUS_ON_NODE=4
mpirun -np $SLURM_CPUS_ON_NODE bin/testJobControl
*/

#include "ThreadsJobControl.hh"
#include "MPIJobControl.hh"
#include "KTAccumJob.hh"
#include <TH1F.h>

/// Test job class interfacing with KeyData
class MyAccumJob: public KTAccumJob {
protected:
    /// operate on kt data to produce accumulables
    void runAccum() override {
        auto f = kt.GetROOT<TH1>("v");
        assert(f);
        for(auto i = JS.N0; i < JS.N1; i++) f->Fill(((i*i)%1000)*0.01);
        printf("Integral %g in ", f->Integral());
        JS.display();
        kt.Set("v", *f);
    }
};

REGISTER_FACTORYOBJECT(MyAccumJob)

/// main() for executable
int main(int argc, char **argv) {

    //MultiJobControl::JC = new ThreadsJobControl();
    MultiJobControl::JC = new MPIJobControl();

    MultiJobControl::JC->verbose = 5;
    MultiJobControl::JC->init(argc, argv);

    if(MultiJobControl::JC->isController()) {

        KTAccumJobComm KTC;
        auto foo = new TH1F("foo","bar",20,0,10);
        KTC.kt.Set("v", *foo);
        KTC.kt.Set("Combine","v");
        KTC.kt.Set("NSamples", 1000);

        KTC.launchAccumulate(typehash<MyAccumJob>());
        for(int i=0; i<10; i++) {
            JobSpec JS;
            JS.uid = i;
            JS.wclass = typehash<JobWorker>();
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