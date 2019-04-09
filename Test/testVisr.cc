/// \file testVisr.cc Test program showing visualizer

#include "Visr.hh"
#include "Icosahedral.hh"
#include "CodeVersion.hh"
#include <TRandom3.h>
#include <stdlib.h>
#include "ColorSpec.hh"

void* visthread(void*) {
    vsr::doGlutLoop();
    return NULL;
}

int main(int, char**) {
    CodeVersion::display_code_version();

    vsr::initWindow("testVisr");

    pthread_t thread;
    pthread_create(&thread, NULL, &visthread, nullptr );
    vsr::pause(); // shows teapot on start

    using namespace Icosahedral;
    vsr::vec3 v1{0.7,0,0};
    vsr::vec3 v2{0.6,0.1,0.1};

    vsr::startRecording(true);
    vsr::clearWindow();

    vsr::setColor(1,0,0,1);
    for(auto& e: points(v2)) vsr::ball(e, 0.02);

    vsr::setColor(0.7,0,0.7,1.0);
    for(auto& e: points(v1)) vsr::ball(e, 0.02);


    using namespace color;

    for(auto& v: {Navigator::v12.c, Navigator::v15.c, Navigator::v20.c}) {
        auto vv = Vec<3,double>(v);
        vv /= vv.mag();
        vsr::ball(vv, 0.05);
    }

    vsr::setColor(1,0,0,1);

    TRandom3 TR;
    for(size_t i=0; i<200; i++) {
        // choose a point on the sphere
        double c = 2*(TR.Uniform()-0.5);
        double s = sqrt(1-c*c);
        double ph = 2*M_PI*TR.Uniform();
        Vec<3,double> v0({s*cos(ph), s*sin(ph), c});
        auto v = v0;

        // map into fundamental domain
        Nav.map_d0(v);
        vsr::ball(v, 0.01);
    }

    vsr::stopRecording();

    vsr::pause();

    vsr::set_kill();
    pthread_join(thread, nullptr);

    return EXIT_SUCCESS;
}
