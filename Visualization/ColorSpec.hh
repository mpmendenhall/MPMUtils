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
#include <stdio.h>

namespace color {

    /// Color specified by rgb(a) components
    template<typename T>
    struct _rgba {
        T r = {};   ///< red component
        T g = {};   ///< green component
        T b = {};   ///< blue component
        T a = {};   ///< alpha channel
        /// Default constructor
        _rgba() { }
        /// Constructor from rgb(a)
        _rgba(T R, T G, T B, T A): r(R), g(G), b(B), a(A) { }
    };

    /// 32-bit color, components in [0,255]
    struct rgb32: public _rgba<uint8_t> {
        /// Default constructor
        rgb32(): _rgba<uint8_t>(0,0,0,255) { }
        /// Constructor from rgb(a)
        rgb32(uint8_t R, uint8_t G, uint8_t B, uint8_t A = 255): _rgba<uint8_t>(R,G,B,A) { }
        /// Color as 24-bit integer 0xRrGgBb
        int32_t as_rgb_i24() const { return (int32_t(r) << 16) + (int32_t(g) << 8) + int32_t(b); }
        /// Color as hexadecimal string "RrGgBb"
        explicit operator string() const { char c[7]; snprintf(c, 7, "%06x", as_rgb_i24()); return c; }
    };

    struct hsv;

    /// Color specified by rgb(a) components in [0,1]
    struct rgb: public _rgba<double> {
        /// Default constructor
        rgb() { }
        /// Constructor from rgb(a)
        rgb(double R, double G, double B, double A = 1): _rgba<double>(R,G,B,A) { }

        /// Auto-convert to HSV, H in [0, 2pi)
        operator hsv() const;
        /// Auto-convert to 32-bit
        operator rgb32() const;
        /// Color as hexadecimal string "RrGgBb"
        explicit operator string() const { return string(rgb32(*this)); }
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

