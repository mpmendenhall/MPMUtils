/// \file SVGGradientAxis.hh Color gradient z-axis for SVG plots
// -- Michael P. Mendenhall, 2018

#ifndef SVGGRADIENTAXIS_HH
#define SVGGRADIENTAXIS_HH

#include "SVGBuilder.hh"
#include "Interval.hh"
#include <set>
using std::set;

/// D-dimensional plane equation
template<size_t D, typename val_tp>
class PlaneEquation {
public:
    /// Constructor
    PlaneEquation() { }
    /// Evaluate at point
    val_tp operator()(const val_tp* x) const {
        val_tp s = 0;
        for(size_t i=0; i<D; i++) s += P[i+1]*(x[i]-x0[i]);
        return s;
    }

    array<val_tp,D> x0{};   ///< relative centerpoint
    array<val_tp,D+1> P{};  ///< coefficients, y = P[0] + P[i+1]*x[i]
};

/// Color axis
class SVGGradientAxis {
public:
    /// Constructor
    SVGGradientAxis();
    /// normalize to axis internal coordinates
    double axisUnits(double x) const;
    /// derivative of axis transformation
    double dAxisUnits(double x) const;
    /// finalize range; set up text
    void finalize();
    /// Determine gradient mapping given face plane equation
    string gradient_remap(const PlaneEquation<2,float>& P) const;
    /// add an axis label tick
    void addtick(double z, const string& lbl = "auto", int lvl = 0);

    struct tick {
        double z;
        int level;
        string label;
        bool operator<(const tick& rhs) const { return z < rhs.z; }
    };

    bool logscale = false;                      ///< log scale setting
    Interval<> range;                           ///< axis range
    set<tick> axticks;                          ///< axis tick locations
    SVG::group* axisGroup = new SVG::group;     ///< group containing axis information
    SVG::lingradient* base_gradient = nullptr;  ///< SVG gradient specification
    color::Gradient G;                          ///< gradient color definition
};

#endif
