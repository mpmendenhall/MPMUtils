/// \file Terminplot.cc

#include "Terminplot.hh"

using namespace Terminart;

void PlotAxis::calc_binedges() {
    binedges.resize(length+1);
    for(int i = 0; i <= length; ++i) binedges[i] = i2x(i);
}
