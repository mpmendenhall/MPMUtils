/// \file VisrSVG.hh SVG 3D visualization driver
// -- Michael P. Mendenhall, 2019

#ifndef VISRSVG_HH
#define VISRSVG_HH

#include "Visr.hh"
#include "Sketch3D.hh"

/// SVG visualization driver
class SVGVisDriver: public VisDriver {
public:

    /// render to file
    void toFile(const string& fname, const Perspective& P = {}) { PL.makeSVG(P, fname); }

    PrimitivesLayer PL; ///< collected drawing objects

protected:

    /// set color for subsequent draws
    void _setColor(const vector<float>&) override;
    /// draw series of lines between vertices
    void _lines(const vector<float>&) override;
    /// draw ball at location
    void _ball(const vector<float>&) override;
    /// clear output
    void _clearWindow(const vector<float>&) override { PL = {}; };

    color::rgb cLine{0.,0.,0.,1.};  ///< line color
    color::rgb cFill{0.,0.,0.,1.};  ///< fill color
};

#endif
