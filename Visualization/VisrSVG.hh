/// \file VisrSVG.hh SVG visualization driver
// Michael P. Mendenhall, LLNL 2019

#ifndef VISRSVG_HH
#define VISRSVG_HH

#include "Visr.hh"
#include "Sketch3D.hh"

/// SVG visualization driver
class SVGVisDriver: public VisDriver {
public:

    /// set color for subsequent draws
    void setColor(const vector<float>&) override;
    /// draw series of lines between vertices
    void lines(const vector<float>&) override;
    /// draw ball at location
    void ball(const vector<float>&) override;
    /// clear output
    void clearWindow(const vector<float>&) override { PL = {}; };

    /// render to file
    void toFile(const string& fname, const Perspective& P = {}) { PL.makeSVG(P, fname); }

    PrimitivesLayer PL;

protected:
    color::rgb cLine{0.,0.,0.,1.};
    color::rgb cFill{0.,0.,0.,1.};
};

#endif
