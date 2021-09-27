/// \file testVisr.cc Test program showing visualizer

#include "ConfigFactory.hh"

#include "VisrGL.hh"
#include "VisrSVG.hh"
#include "Icosahedral.hh"

#include <TRandom3.h>
#include <stdlib.h>
#include <stdexcept>

REGISTER_EXECLET(testVisr) {

    GLVisDriver theGLDr;    ///< OpenGL-based visualization driver
    SVGVisDriver theSVGDr;  ///< SVG output
    VisDriver* theVis = nullptr;

    if(GLVisDriver::hasGL) {
        theGLDr.scale = 0.1;
        theVis = &theGLDr;
        theGLDr.windowTitle = "MPMUtils Visualizer Test";
        theGLDr.doGlutLoop();
        theGLDr.display();
    }
    if(!theVis) throw std::logic_error("no valid visualizer");

    theVis->pause(); // shows teapot on start

    theVis->startRecording(true);

    theVis->setColor(1,0,0,1);

    // compatible type between Icosahedral matrix operations and VisDriver::vec3
    typedef Vec<3,double> vec3;
    vec3 v1{{2,0,0}};
    for(auto& e: Icosahedral::points(v1)) theVis->ball(e, 0.1);

    //using namespace color;

    /*
    theVis->setColor(0.7,0,0.7,1.0);
    for(auto& v: {Navigator::v12.c, Navigator::v15.c, Navigator::v20.c}) {
        auto vv = Vec<3,double>(v);
        vv /= vv.mag();
        theVis->ball(vv, 0.05);
    }

    for(auto& g: Rs) {

        theVis->setColor(0,0,1,1);
        bcoord_t<double> bc0({0.1,0.,1.});
        theVis->startLines(true);
        for(auto& m: Navigator::v12.R) {
            bc0.n = m.i;
            auto v = Matrix<3,3,double>(g)*bc0.v();
            theVis->vertex(v/v.mag());
        }
        theVis->endLines();

        theVis->setColor(0,1,0,1);
        bc0 = bcoord_t<double>({1.,0.,0.1});
        theVis->startLines(true);
        for(auto& m: Navigator::v20.R) {
            bc0.n = m.i;
            auto v = Matrix<3,3,double>(g)*bc0.v();
            theVis->vertex(v/v.mag());
        }
        theVis->endLines();

    }

    theVis->setColor(1,0,0,1);

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
        theVis->ball(v, 0.01);
    }
    */

    theVis->stopRecording();

    theVis->pause();

}
