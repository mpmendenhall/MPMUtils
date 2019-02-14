/// \file testJobControl.cc Test of job control classes

#include "ThreadsJobControl.hh"
#include "KTAccumulateJobComm.hh"

/// Test job class interfacing with KeyData
class MyAccumJob: public KTAccumJob {
protected:
    /// operate on kt data to produce accumulables
    void run(JobSpec S) {
        S.display();
        for(auto& kv: kt) {
            printf("\tKey %s\n", kv.first.c_str());
        }
    }
};

REGISTER_FACTORYOBJECT(MyAccumJob)

/// main() for executable
int main(int argc, char **argv) {

    MultiJobControl::JC = new ThreadsJobControl();
    MultiJobControl::JC->verbose = 5;
    MultiJobControl::JC->init(argc, argv);

    if(MultiJobControl::JC->rank == 0) {
        KTAccumulateJobComm KTC;

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
        MultiJobControl::JC->waitComplete();
        printf("\n\nAll done!\n");
    } else MultiJobControl::JC->runWorker();

    MultiJobControl::JC->finish();

    return EXIT_SUCCESS;
}
