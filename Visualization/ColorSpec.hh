/// \file ColorSpec.hh Colors and gradients specified in rgba or hsva
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
        double r;   ///< red component, in [0,1]
        double g;   ///< green component, in [0,1]
        double b;   ///< blue component, in [0,1]
        double a;   ///< alpha channel, in [0,1]

        /// Default constructor
        rgb(): r(0), g(0), b(0), a(0) { }
        /// Constructor from rgb(a)
        rgb(double R, double G, double B, double A = 1): r(R), g(G), b(B), a(A) { }

        /// Auto-convert to HSV, H in [0, 2pi)
        operator hsv() const;
        /// Color as 24-bit integer 0xRrGgBb
        int32_t as_rgb_i24() const;
        /// Color as hexadecimal string "RrGgBb"
        explicit operator string() const;
    };

    /// Construct color with normalization to [0,1]
    template<size_t n = 255>
    rgb rgbn(double R, double G, double B, double A = n) { return rgb(R/n, G/n, B/n, A/n); }

    /// Color specified by hsv(a) components
    struct hsv {
        double h;       ///< hue angle (radians; unnormalized angular range)
        double s;       ///< saturation, in [0,1]
        double v;       ///< value, in [0,1]
        double a;       ///< alpha channel, in [0,1]

        /// Default constructor
        hsv(): h(0), s(0), v(0), a(0) { }
        /// Constructor from hsv(a)
        hsv(double H, double S, double V, double A=1): h(H), s(S), v(V), a(A) { }

        /// Auto-convert to RGB
        operator rgb() const;
    };

    /// Color gradient generator, defined by color values at stops
    class Gradient: public map<double, pair<rgb,hsv>> {
    public:
        /// Inherit map constructors
        using map<double, pair<rgb,hsv>>::map;

        /// add rgb color stop
        void addStop(double x, const rgb& c) { (*this)[x] = {c, hsv(c)}; }
        /// add hsv color stop
        void addStop(double x, const hsv& c) { (*this)[x] = {rgb(c), c}; }

        /// linear rgb component interpolation
        rgb rgbcolor(double x) const;
        /// linear hsv component interpolation
        hsv hsvcolor(double x) const;

        /// find relative position in [0,1] between stops bracketing point
        double findPoint(double x, const_iterator& it0,  const_iterator& it1) const;
    };
}

#endif

