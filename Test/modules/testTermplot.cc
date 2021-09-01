/// \file testTermplot.cc Test ASCII-art plotter
// Michael P. Mendenhall

#include "ConfigFactory.hh"
#include "Terminplot.hh"
#include <cmath>
#include <iostream>

using namespace Terminart;

REGISTER_EXECLET(testTermplot) {
    TermGraph TG;
    for(int i = 0; i < 30; ++i) TG.emplace_back(i, sin(i*0.12321*M_PI));
    //TG.displayTable();
    TG.autorange();

    std::cout << TG.toArray().render();

    //pixelarray_t a({30,80});
    //TG.getView({0,0}, a);
    //std::cout << a.render("#\n");
}
