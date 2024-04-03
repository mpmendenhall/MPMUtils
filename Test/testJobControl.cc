/// @file testJobControl.cc Test of job control classes

/*
export SLURM_CPUS_ON_NODE=4
mpirun -np $SLURM_CPUS_ON_NODE bin/testJobControl
*/

#include "MPIJobControl.hh"
#include "ThreadsJobControl.hh"
#include "KTAccumJob.hh"
#include "CodeVersion.hh"
#include "JobState.hh"
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

REGISTER_FACTORYOBJECT(MyAccumJob, JobWorker)

/// Local-side specification
class MyJobComm: public KTAccumJobComm {
public:
    /// get correct worker class ID
    string workerType() const override { return "MyAccumJob"; }
};

bool MPI_generic_init(int argc, char **argv) {
    MPIBinaryIO::init(argc,argv);
    MPIBinaryIO::display();

    if(MPIBinaryIO::mpisize <= 1) {
        // single MPI node? configure to run locally
        auto LJC = new LocalJobControl();
        MultiJobControl::JC = LJC;
        LocalJobControl::JW = LJC;
    } else if(!MPIBinaryIO::mpirank) {
        // on MPI controller node?
        MultiJobControl::JC = new MPIJobControl();

    } else {
        // on MPI worker node?
        MultiJobWorker::JW = new MPIJobWorker();
        MultiJobWorker::JW->verbose = 5;
        JobState::stateDir = "./SavedState/";
        MultiJobWorker::JW->runWorkerJobs();

        delete MultiJobWorker::JW;
        MPIBinaryIO::uninit();
        return false;
    }
    return true;
}

/// main() for executable
int main(int argc, char **argv) {
    CodeVersion::display_code_version();

    // exit here after completing jobs on worker node
    if(!MPI_generic_init(argc,argv)) return EXIT_SUCCESS;

    MultiJobControl::JC->verbose = 5;
    printf("\n---- Submitting 10 do-nothing jobs ----\n\n");
    for(int i=0; i<10; i++) {
        JobSpec JS;
        JS.uid = i;
        JS.wclass = "JobWorker";
        MultiJobControl::JC->submitJob(JS);
    }
    printf("\n\nAll submitted!\n\n");

    // wait for them all to complete
    MultiJobControl::JC->waitComplete();
    printf("----- *** -----\n\n");

    // accumulate 1000 counts of events, spread over however many jobs available
    printf("----- Launching accumulation jobs ------\n");
    MyJobComm KTC;
    auto foo = new TH1F("foo","bar",20,0,10);
    KTC.kt.Set("v", *foo);
    KTC.kt.Set("Combine","v");
    KTC.kt.Set("NSamples", 1000);

    KTC.launchAccumulate();
    printf("\n-- Accumulator jobs all launched. --\n\n");

    MultiJobControl::JC->waitComplete();
    printf("\n\nAll done!\n");

    KTC.gather();
    auto f = KTC.kt.GetROOT<TH1>("v");
    if(!f) throw std::logic_error("missing accumulated histogram\n");
    for(int i=1; i<=f->GetNbinsX(); i++) printf("\t%i\t%g\n", i, f->GetBinContent(i));

    // cleanup --- will send end command to remote jobs
    delete MultiJobControl::JC;
    MPIBinaryIO::uninit();
    return EXIT_SUCCESS;
}
