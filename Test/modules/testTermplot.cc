/// \file testTermplot.cc Test ASCII-art plotter
// Michael P. Mendenhall

#include "ConfigFactory.hh"
#include "Terminplot.hh"
#include <cmath>
#include <iostream>
#include <TRandom3.h>
#include <time.h>

using namespace Terminart;

TRandom3 TR;

void gSin(int npts, double phi = 0, double k = 5.2) {
    TermGraph TG;
    for(int i = 0; i < npts; ++i) TG.emplace_back(i, sin(i*k*M_PI/npts + phi) + TR.Gaus()*sqrt(npts)*5e-3);
    TG.autorange();

    auto a = TG.toArray();
    std::cout << a.render() << cmove_control(-a.dim);
}

REGISTER_EXECLET(testTermplot) {
    double phi = 0;
    while(1) {
        gSin(600, phi);
        phi += 0.1;
        usleep(10000);
    }
}
