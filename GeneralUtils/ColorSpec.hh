// This file was produced under the employ of the United States Government,
// and is consequently in the PUBLIC DOMAIN, free from all provisions of
// US Copyright Law (per USC Title 17, Section 105).
//
// -- Michael P. Mendenhall, 2015

#ifndef COLORSPEC_HH
#define COLORSPEC_HH

#include <map>
using std::map;
using std::pair;
#include <string>
using std::string;
#include <cstdint>

namespace color {
    
    struct hsv;
    
    /// Color specified by rgb(a) components
    struct rgb {
        double r;       ///< red component, in [0,1]
        double g;       ///< green component, in [0,1]
        double b;       ///< blue component, in [0,1]
        double a;       ///< alpha channel, in [0,1]
        
        /// Constructor
        rgb(): r(0), g(0), b(0), a(0) { }
        /// Constructor from rgb(a)
        rgb(double R, double G, double B, double A=1): r(R), g(G), b(B), a(A) { }
        /// Constructor from hsv specification
        rgb(const hsv& c);
        
        /// Color as 0xRgGgBb 24-bit number
        int32_t as24bit() const;
        /// Color as RgGgBb hexadecimal string
        string asHexString() const;
    };

    /// Color specified by hsv(a) components
    struct hsv {
        double h;       ///< hue, in [0,2*pi)
        double s;       ///< saturation, in [0,1]
        double v;       ///< value, in [0,1]
        double a;       ///< alpha channel, in [0,1]
        
        /// Constructor
        hsv(): h(0), s(0), v(0), a(0) { }
        /// Constructor from hsv(a)
        hsv(double H, double S, double V, double A=1): h(H), s(S), v(V), a(A) { }
        /// Constructor from rgb specification
        hsv(const rgb& c);
    };
    
    /// Color gradient generator, defined by color values at stops
    class Gradient {
    public:
        /// Constructor
        Gradient() { }
        
        /// add rgb color stop
        void addStop(double x, const rgb& c) { stops[x] = pair<rgb,hsv>(c, hsv(c)); }
        /// add hsv color stop
        void addStop(double x, const hsv& c) { stops[x] = pair<rgb,hsv>(rgb(c), c); }
        
        /// linear rgb component interpolation
        rgb rgbcolor(double x) const;
        /// linear hsv component interpolation
        hsv hsvcolor(double x) const;
        
        /// Create a new gradient as a sub-range of this one
        Gradient subGradient(double x0, double x1) const;
        
        /// gradient stops map access
        const map<double,pair<rgb,hsv> >& getStops() const { return stops; }
        
    protected:
        map<double,pair<rgb,hsv> > stops;       ///< gradient stops
        typedef  map<double,pair<rgb,hsv> >::const_iterator stopit;
        /// stop-locating utility function
        double findPoint(double x, stopit& it0,  stopit& it1) const;
    };
}

#endif

