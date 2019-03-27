/// \file testVisr.cc Test program showing visualizer

#include "Visr.hh"
#include "Icosahedral.hh"
#include "CodeVersion.hh"
#include <stdlib.h>

void* visthread(void*) {
    vsr::doGlutLoop();
    return NULL;
}

int main(int, char**) {
    CodeVersion::display_code_version();

    vsr::initWindow("testVisr");

    pthread_t thread;
    pthread_create(&thread, NULL, &visthread, nullptr );

    using namespace Icosahedral;
    vsr::vec3 v1{0.7,0,0};
    vsr::vec3 v2{0.6,0.1,0.1};

    vsr::startRecording();
    vsr::setColor(0.7,0,0.7,1.0);
    for(auto& e: points(v1)) vsr::ball(e, 0.02);
    vsr::setColor(1,0,0,1);
    for(auto& e: points(v2)) vsr::ball(e, 0.02);
    vsr::stopRecording();

    vsr::pause();

    vsr::set_kill();
    pthread_join(thread, nullptr);

    return EXIT_SUCCESS;
}
