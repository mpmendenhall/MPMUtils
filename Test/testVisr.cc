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

    using namespace Icosahedral;
    vsr::vec3 v1{0.7,0,0};
    vsr::vec3 v2{0.6,0.1,0.1};

    vsr::startRecording();
    vsr::setColor(0.7,0,0.7,1.0);
    for(auto& e: points(v1)) vsr::ball(e, 0.02);
    vsr::setColor(1,0,0,1);
    for(auto& e: points(v2)) vsr::ball(e, 0.02);

    using namespace color;

    TRandom3 TR;
    for(size_t i=0; i<200; i++) {
        double c = 2*(TR.Uniform()-0.5);
        double s = sqrt(1-c*c);
        double ph = 2*M_PI*TR.Uniform();
        vsr::vec3 v0{s*cos(ph), s*sin(ph), c};

        Nav.map_d0(v0);
        vsr::ball(v0, 0.01);
        continue;

        for(auto& v: points(v0)) {
            auto dmn = Nav.domain(v);
            auto det = SqMat<3>::det(Rs.element(dmn));

            if(det > 0) vsr::setColor(1,0,0);
            else vsr::setColor(0,0,1);

            //rgb cl(hsv(2*/120., 1, 1));
            //vsr::setColor(cl.r, cl.g, cl.b, 1);

            vsr::ball(v, 0.01);
        }
    }

    vsr::stopRecording();

    vsr::pause();

    vsr::set_kill();
    pthread_join(thread, nullptr);

    return EXIT_SUCCESS;
}
