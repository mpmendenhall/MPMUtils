/// \file testJobControl.cc Test of job control classes

#include "ThreadsJobControl.hh"

/// main() for executable
int main(int argc, char **argv) {

    MultiJobControl::JC = new ThreadsJobControl();
    MultiJobControl::JC->verbose = 5;
    MultiJobControl::JC->init(argc, argv);

    if(MultiJobControl::JC->rank == 0) {
        for(int i=0; i<10; i++) {
            JobSpec JS;
            JS.uid = i;
            JS.wclass = typehash<JobWorker>();
            MultiJobControl::JC->submitJob(JS);
        }
        MultiJobControl::JC->waitComplete();
        printf("\n\nAll done!\n");
    } else MultiJobControl::JC->runWorker();

    MultiJobControl::JC->finish();

    return EXIT_SUCCESS;
}
