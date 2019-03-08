/// \file testVisr.cc Test program showing visualizer

#include "Visr.hh"
#include "Icosahedral.hh"
#include <stdlib.h>

void* visthread(void*) {
    vsr::doGlutLoop();
    return NULL;
}

int main(int, char**) {

    std::cout << Icosahedral::Rs.size() << " icosahedral rotations\n";
    for(auto& m: Icosahedral::Rs) std::cout << m << "\n";

    vsr::initWindow("testVisr");

    pthread_t thread;
    pthread_create(&thread, NULL, &visthread, nullptr );

    for(int i=1; i<11; i++) {
        vsr::startRecording();
        vsr::setColor(0,0,0.7,0.2);
        vsr::setWireframe(i%2);
        vsr::teapot(0.1*i);
        vsr::setColor(0.7,0,0.7,1.0);
        vsr::ball({0.5,0.3,0.2*i}, 0.01*i);
        vsr::stopRecording();
        vsr::pause();
    }

    vsr::set_kill();
    pthread_join(thread, nullptr);

    return EXIT_SUCCESS;
}
