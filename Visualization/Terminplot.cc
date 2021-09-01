/// \file Terminplot.cc

#include "Terminplot.hh"
#include <algorithm>
#include <stdexcept>
#include <cmath>

using namespace Terminart;

void PlotAxis::calc_binedges() {
    binedges.resize(length+1);
    for(int i = 0; i <= length; ++i) binedges[i] = i2x(i);
}

void PlotAxis::getView(rowcol_t x0, pixelarray_t& a, const Compositor& C) const {
    if(horizontal) a.hline(x0, length, {'-'}, C);
    else a.vline({x0.first, x0.second + hpad}, length, {'|'}, C);
}

//---------------------------------

void TermGraph::initAxes() {
    if(!Ax) Ax = new LinAxis(true, 0, 0, 80);
    if(!Ay) Ay = new LinAxis(false,0, 0, 15);
    Ax->horizontal = true;
    Ay->horizontal = false;
}

void TermGraph::autorange() {
    initAxes();
    if(!size()) return;

    auto mmx = std::minmax_element(begin(), end());
    Ax->autorange(mmx.first->first, mmx.second->first);

    auto mmy = std::minmax_element(begin(), end(), [](auto a, auto b) { return a.second < b.second; });
    Ay->autorange(mmy.first->second, mmy.second->second);
}

void TermGraph::displayTable() const {
    for(auto xy: *this) printf("%12g\t%12g\n", xy.first, xy.second);
}

rectangle_t TermGraph::getBounds() const {
    if(!Ax || !Ay) return null_rectangle;
    auto bb = Ay->getBounds();
    bb.include(Ax->getBounds().second + bb.second);
    return bb;
}

void TermGraph::getView(rowcol_t p0, pixelarray_t& v, const Compositor& C) const {
    if(!Ax || !Ay) throw std::logic_error("Uninitialized axes for graph view");
    if(Ay->horizontal || !Ax->horizontal) throw std::logic_error("Inconsistent axis orientations for graph view");

    Ay->getView(p0, v, C);
    p0 = p0 + Ay->getBounds().second;
    Ax->getView(p0, v, C);
    v.cput(p0 - rowcol_t(0,1), {'+'}, C);

    for(auto xy: *this) {
        auto ix = Ax->x2i(xy.first);
        auto iy = -Ay->x2i(xy.second) - 1;
        int ii = symbs.size()*(round(iy) - iy + 0.5);
        if(ii == (int)symbs.size()) --ii;
        v.cput(p0 + rowcol_t(round(iy), round(ix)), {symbs[ii]}, C);
    }
}
