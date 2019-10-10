/// \file VisrSVG.cc
// Michael P. Mendenhall, LLNL 2019

#include "VisrSVG.hh"

void SVGVisDriver::_setColor(const vector<float>& v) {
    cLine = cFill = color::rgb(v[0],v[1],v[2],v[3]);
}

void SVGVisDriver::_lines(const vector<float>& v) {
    if(v.size() < 7) return;

    PL.myObjs.push_back(make_unique<ProjectablePoly>());
    auto& P = *static_cast<ProjectablePoly*>(PL.myObjs.back().get());

    for(size_t i = 0; i+3 < v.size(); i += 3) {
        xyzpt p{(double)v[i], (double)v[i+1], (double)v[i+2]};
        P.pts.push_back(p);
    }

    if(v.back()) P.pts.push_back(P.pts[0]);
    SVG::set_stroke(P.attrs, cLine);
}

void SVGVisDriver::_ball(const vector<float>& v) {
    xyzpt p{(double)v[0], (double)v[1], (double)v[2]};
    PL.myObjs.push_back(std::unique_ptr<ProjectableBall>(new ProjectableBall(p, v[3])));
    auto& P = *PL.myObjs.back().get();
    SVG::set_fill(P.attrs, cLine);
}
