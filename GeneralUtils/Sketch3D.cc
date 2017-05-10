/// \file Sketch3D.cc
// Michael P. Mendenhall, LLNL 2017

#include "Sketch3D.hh"
#include <algorithm>

void Perspective::project(const double xyz[3], double xyzs[4]) const {
    // position relative to viewer
    for(auto i: {0,1,2}) xyzs[i] = M[i][0]*xyz[0] + M[i][1]*xyz[1] + M[i][2]*xyz[2] - v0[i];
    if(flipY) xyzs[1] *= -1;

    // perspective scaling factor
    if(isOrtho) xyzs[3] = 1.;
    else {
        xyzs[3] = -v0[2]/xyzs[2];
        xyzs[0] *= xyzs[3];
        xyzs[1] *= xyzs[3];
    }
}

void Perspective::clearRotation() {
    for(auto i: {0,1,2})
        for(auto j: {0,1,2})
            M[i][j] = i==j;
}

void Perspective::projectPoly(const vector<xyzpt>& vIn, vector<SVG::xypoint>& vOut, double& s, double& z) const {
    s = z = 0;
    xyzspt pp;
    for(auto& p: vIn) {
        project(p,pp);
        SVG::xypoint ppp{pp[0],pp[1]};
        vOut.push_back(ppp);
        z += pp[2];
        s += pp[3];
    }
    s /= vIn.size();
    z /= vIn.size();
}

////////////////////////////////
////////////////////////////////

void ProjectableBall::setPerspective(const Perspective& P) {
    xyzspt cp;
    P.project(c,cp);
    z = cp[2];
    s = cp[3];
    myXML = make_shared<SVG::circle>(cp[0], cp[1], fabs(s*r));
    setAttrs();
}

void ProjectablePoly::setPerspective(const Perspective& P) {
    auto pg = make_shared<SVG::polyline>();
    P.projectPoly(pts, pg->pts, s, z);
    if(closed) pg->name = "polygon";
}

/////////////////////////////////
/////////////////////////////////

void SketchLayer::makeSVG(const Perspective& P, const string& fname, double xborder, const string& ttl) {
    SVG::SVGDoc D;
    if(ttl.size()) D.body.addChild(make_shared<SVG::title>(ttl));
    drawInto(D.body, P);
    D.BB = D.body.getBB();
    D.BB.expand(xborder);
    D.write(fname);
}

void SketchLayer::makeStereo(Perspective& P, const string& fname, double xborder, const string& ttl) {
    auto g1 = make_shared<SVG::group>();
    auto g2 = make_shared<SVG::group>();

    drawInto(*g1, P);
    P.v0[0] *= -1;
    drawInto(*g2, P);
    P.v0[0] *= -1;

    auto BB1 = g1->getBB();
    auto BB2 = g2->getBB();
    BB1.expand(xborder);
    BB2.expand(xborder);
    std::array<double,2> dx1 = {xborder-BB1.lo[0], 0};
    std::array<double,2> dx2 = {-BB2.hi[0]-xborder, 0};
    BB1.offset(dx1);
    BB2.offset(dx2);
    g1->attrs["transform"] = "translate("+SVG::to_str(dx1[0])+")";
    g2->attrs["transform"] = "translate("+SVG::to_str(dx2[0])+")";

    SVG::SVGDoc D;
    if(ttl.size()) D.body.addChild(make_shared<SVG::title>(ttl));
    D.body.addChild(g1);
    D.body.addChild(g2);
    D.BB += BB1;
    D.BB += BB2;

    D.write(fname);
}

//////////////////////////////////////

void MultiLayer::drawInto(XMLBuilder& X, const Perspective& P) {
    for(auto l: myLayers) {
        auto g = make_shared<SVG::group>();
        l->drawInto(*g, P);
        X.addChild(g);
    }
}

//////////////////////////////////////

bool compare_projectables(const unique_ptr<ProjectablePrimitive>& lhs,
                          const unique_ptr<ProjectablePrimitive>& rhs) { assert(lhs && rhs); return lhs->z < rhs->z; }

void PrimitivesLayer::drawInto(XMLBuilder& X, const Perspective& P) {
    for(auto& o: myObjs) { assert(o); o->setPerspective(P); }
    std::sort(myObjs.begin(), myObjs.end(), &compare_projectables);
    for(auto& o: myObjs) X.addChild(o->myXML);
}
