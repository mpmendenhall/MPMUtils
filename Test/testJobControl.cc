/// \file testJobControl.cc Test of job control classes

#include "ThreadsJobControl.hh"
#include "KTAccumJob.hh"

/// Test job class interfacing with KeyData
class MyAccumJob: public KTAccumJob {
protected:
    /// operate on kt data to produce accumulables
    void run(JobSpec S) {
        S.display();
        auto k = kt.FindKey("v", true);
        assert(k);
        auto n = k->vSize<double>();
        auto v = k->GetPtr<double>();
        printf("Vector of size %i\n", n);
        while(n--) {
            v[n] = 2;
            usleep(100000);
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
        KTAccumJobComm KTC;
        vector<double> v(10);
        KTC.kt.Set("v", v);
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
        MultiJobControl::JC->waitComplete();
        printf("\n\nAll done!\n");

        KTC.gather();
        auto k = KTC.kt.FindKey("v", true);
        assert(k);
        auto n = k->vSize<double>();
        auto p = k->GetPtr<double>();
        printf("Vector of size %i\n", n);
        while(n--) printf("\t%g\n", p[n]);

    } else MultiJobControl::JC->runWorker();

    MultiJobControl::JC->finish();

    return EXIT_SUCCESS;
}
