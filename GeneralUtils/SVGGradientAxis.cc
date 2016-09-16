/// \file SVGGradientAxis.cc
// This file was produced under the employ of the United States Government,
// and is consequently in the PUBLIC DOMAIN, free from all provisions of
// US Copyright Law (per USC Title 17, Section 105).
//
// -- Michael P. Mendenhall, 2015

#include "SVGGradientAxis.hh"
#include <cassert>

SVGGradientAxis::SVGGradientAxis() {
    // rainbow gradient scale bar
    int ngradstops = 6;
    for(int i=0; i<ngradstops; i++) {
        double l = double(i)/(ngradstops-1);
        G.addStop(l, color::hsv((1-l)*1.5*M_PI,1,1));
    }

    // base gradient
    base_gradient = make_shared<SVG::lingradient>(G, "zaxis", 0, 0, 1, 0);
    base_gradient->attrs["gradientUnits"] = "userSpaceOnUse";

    // derived axis gradient
    Gaxis->attrs["id"] = "Gaxis";
    Gaxis->attrs["gradientTransform"] = "rotate(-90) translate(-1 0)";
    Gaxis->attrs["xlink:href"] = "#"+base_gradient->attrs["id"];
    axisGroup->addChild(Gaxis);

    // gradient rectangle
    auto r = make_shared<SVG::rect>(0, 0, 0.1, 1);
    r->attrs["style"] = "fill:url(#" + Gaxis->attrs["id"] + ");stroke:black;stroke-width:0.002";
    axisGroup->addChild(r);

    axisGroup->attrs["font-size"] = "0.07";
}

double SVGGradientAxis::axisUnits(double x) const {
    if(logscale) return x > 0? log(x/range.lo[0]) / log(range.hi[0]/range.lo[0]) : -100;
    return (x-range.lo[0])/range.dl(0);
}

double SVGGradientAxis::dAxisUnits(double) const {
    assert(!logscale);
    return 1./range.dl(0);
}

void SVGGradientAxis::finalize() {
    if(logscale) {
        if(range.lo[0] < 1e-6*range.hi[0]) range.lo[0] = 1e-6*range.hi[0];
    }

    if(!axticks.size()) {
        addtick(range.lo[0]);
        addtick(range.hi[0]);
    }

    for(auto const& t: axticks) {
        double zz = 1-axisUnits(t.z);
        if(zz < 0 || zz > 1) continue;
        auto tln = make_shared<SVG::line>(0.1-0.03*pow(0.66,t.level), zz, .1 + 0.01*pow(0.5,t.level), zz, "stroke:black;stroke-width:"+to_str(0.005*pow(2,-t.level)));
        axisGroup->addChild(tln);
        if(!t.label.size()) continue;
        if(zz < 0.06) zz = 0.06;
        else if(zz > 0.995) zz = 0.995;
        else zz += 0.025;        // ugly manual tweak to center text
        auto tktxt = make_shared<SVG::text>(t.label, 0.115, zz);
        //tktxt->attrs["dominant-baseline"]="middle"; // unfortunately unsupported
        axisGroup->addChild(tktxt);
    }
}

string SVGGradientAxis::gradient_remap(const PlaneEquation<2,float>& P) const {
    double gx = dAxisUnits(P.P[1])*P.P[1];
    double gy = dAxisUnits(P.P[2])*P.P[2];
    double th = atan2(gy,gx)*180./M_PI;     // rotation angle [degrees]
    double mg2 = gx*gx + gy*gy;             // magnitude of gradient
    string txstr = "translate("+to_str(P.x0[0])+","+to_str(P.x0[1])+") ";
    txstr += "rotate("+to_str(th)+") ";
    txstr += "scale("+to_str(1./sqrt(mg2))+") ";
    txstr += "translate("+to_str(-axisUnits(P.P[0]))+",0)";
    return txstr;
}

void SVGGradientAxis::addtick(double z, const string& lbl, int lvl) {
    tick t;
    t.z = z;
    if(lbl == "auto") t.label = to_str(z);
    else t.label = lbl;
    t.level = lvl;
    axticks.insert(t);
}
