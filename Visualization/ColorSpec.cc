/// \file ColorSpec.cc
// This file was produced under the employ of the United States Government,
// and is consequently in the PUBLIC DOMAIN, free from all provisions of
// US Copyright Law (per USC Title 17, Section 105).
//
// -- Michael P. Mendenhall, 2015

#include "ColorSpec.hh"
#include <cmath>
#include <stdexcept>

namespace color {

    rgb::operator hsv() const {
        hsv C(0, 0, std::max(r, std::max(g, b)), a);
        double d = C.v - std::min(r, std::min(g, b));
        if(d == 0) return C;

        C.s = d/C.v;
        if(C.v == r) C.h = (g - b)/d;
        else if(C.v == g) C.h = 2 + (b - r)/d;
        else C.h = 4 + (r - g)/d;
        C.h *= M_PI/3;
        return C;
    }

    rgb::operator rgb32() const {
        int rr = r <= 0? 0 : 256*r;
        if(rr > 255) rr = 255;
        int gg = g <= 0? 0 : 256*g;
        if(gg > 255) gg = 255;
        int bb = b <= 0? 0 : 256*b;
        if(bb > 255) bb = 255;
        int aa = a <= 0? 0 : 256*a;
        if(aa > 255) aa = 255;
        return rgb32(rr,gg,bb,aa);
    }

    hsv::operator rgb() const {
        if(s == 0) return rgb(v,v,v,a);

        double hh = (h < 0)? 2*M_PI - fmod(fabs(h), 2*M_PI) : fmod(h, 2*M_PI);
        double H = 3*hh/M_PI;
        int I = (int)floor(H);
        double X = v * ( 1 - s );
        double Y = v * ( 1 - s * ( H - I ));
        double Z = v * ( 1 - s * ( 1 - ( H - I )));
        switch(I) {
            case 0: return rgb(v, Z, X, a);
            case 1: return rgb(Y, v, X, a);
            case 2: return rgb(X, v, Z, a);
            case 3: return rgb(X, Y, v, a);
            case 4: return rgb(Z, X, v, a);
            case 5: return rgb(v, X, Y, a);
            default: abort();
        }
    }

    //-----------------------------------------
    //-----------------------------------------

    double Gradient::findPoint(double x, const_iterator& it0,  const_iterator& it1) const {
        if(!size()) throw std::runtime_error("Querying null gradient");
        if(size() == 1) { it0 = it1 = begin(); return 0.5; }

        it1 = lower_bound(x);
        if(it1 == end()) --it1;
        if(it1 == begin()) ++it1;
        it0 = std::prev(it1);

        double x0 = it0->first;
        double x1 = it1->first;
        return (x-x0)/(x1-x0);
    }

    rgb Gradient::rgbcolor(double x) const {
        if(!size()) return rgb();

        const_iterator it0, it1;
        double l1 = findPoint(x, it0, it1);
        double l0 = 1-l1;
        const rgb& c0 = it0->second.first;
        const rgb& c1 = it1->second.first;

        return rgb( l0*c0.r + l1*c1.r,
                    l0*c0.g + l1*c1.g,
                    l0*c0.b + l1*c1.b,
                    l0*c0.a + l1*c1.a );
    }

    hsv Gradient::hsvcolor(double x) const {
        if(!size()) return hsv();

        const_iterator it0, it1;
        double l1 = findPoint(x, it0, it1);
        double l0 = 1-l1;
        const hsv& c0 = it0->second.second;
        const hsv& c1 = it1->second.second;

        return hsv( l0*c0.h + l1*c1.h,
                    l0*c0.s + l1*c1.s,
                    l0*c0.v + l1*c1.v,
                    l0*c0.a + l1*c1.a );
    }
}


