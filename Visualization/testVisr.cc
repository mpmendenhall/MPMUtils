/// \file testVisr.cc Test program showing visualizer

#include "Visr.hh"
#include <stdlib.h>

void* visthread(void*) {
    vsr::doGlutLoop();
    return NULL;
}

int main(int, char**) {
    vsr::initWindow("testVisr");

    pthread_t thread;
    pthread_create(&thread, NULL, &visthread, nullptr );

    for(int i=0; i<10; i++) {
        vsr::startRecording();
        vsr::teapot(0.1*(i+1));
        vsr::dot({0.5,0.3,0.2*i});
        vsr::stopRecording();
        vsr::pause();
    }

    vsr::set_kill();
    pthread_join(thread, nullptr);

    return EXIT_SUCCESS;
}
