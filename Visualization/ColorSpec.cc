/// \file ColorSpec.cc
// This file was produced under the employ of the United States Government,
// and is consequently in the PUBLIC DOMAIN, free from all provisions of
// US Copyright Law (per USC Title 17, Section 105).
//
// -- Michael P. Mendenhall, 2015

#include "ColorSpec.hh"
#include <algorithm>
#include <cmath>
#include <cassert>
#include <stdio.h>

namespace color {

    rgb::rgb(const hsv& c) {
        a = c.a;

        if(c.s==0) {
            r = g = b = c.v;
            return;
        }

        double h = c.h < 0? 2*M_PI-fmod(fabs(c.h),2*M_PI) : fmod(c.h, 2*M_PI);
        double var_h = 3*h/M_PI;
        int var_i = (int)floor(var_h);
        double var_1 = c.v * ( 1 - c.s );
        double var_2 = c.v * ( 1 - c.s * ( var_h - var_i ));
        double var_3 = c.v * ( 1 - c.s * ( 1 - ( var_h - var_i )));
        switch(var_i) {
            case 0:
                r = c.v;
                g = var_3;
                b = var_1;
                return;
            case 1:
                r = var_2;
                g = c.v;
                b = var_1;
                return;
            case 2:
                r = var_1;
                g = c.v;
                b = var_3;
                return;
            case 3:
                r = var_1;
                g = var_2;
                b = c.v;
                return;
            case 4:
                r = var_3;
                g = var_1;
                b = c.v;
                return;
            case 5:
                r = c.v;
                g = var_1;
                b = var_2;
                return;
        }
    }

    int32_t rgb::as24bit() const {
        int32_t rr = r <= 0? 0 : 256*r;
        if(rr > 255) rr = 255;
        int32_t gg = g <= 0? 0 : 256*g;
        if(gg > 255) gg = 255;
        int32_t bb = b <= 0? 0 : 256*b;
        if(bb > 255) bb = 255;
        return (rr << 16) + (gg << 8) + bb;
    }

    string rgb::asHexString() const {
        char c[7];
        sprintf(c,"%06x",as24bit());
        return c;
    }

    hsv::hsv(const rgb& c) {
        a = c.a;
        v = std::max(c.r,std::max(c.g, c.b));
        double d = v - std::min(c.r, std::min(c.g, c.b));
        if(d==0) { h=0; s=0; return; }

        s = d/v;
        if(v == c.r) h = (c.g - c.b)/d;
        else if(v == c.g) h = 2 + (c.b -c.r)/d;
        else h = 4 + (c.r - c.g)/d;
        h *= M_PI/3;
    }


    ///////////////////////////////////////////


    double Gradient::findPoint(double x, stopit& it0,  stopit& it1) const {
        if(!stops.size()) { it0 = it1 = stops.end(); return 0; }
        if(stops.size()==1) { it0 = it1 = stops.begin(); return 0; }

        it1 = stops.lower_bound(x);
        if(it1 == stops.end()) it1--;
        if(it1 == stops.begin()) it1++;

        it0 = it1;
        it0--;

        double x0 = it0->first;
        double x1 = it1->first;
        return (x-x0)/(x1-x0);
    }

    rgb Gradient::rgbcolor(double x) const {
        if(!stops.size()) return rgb();

        stopit it0, it1;
        double l1 = findPoint(x, it0, it1);
        double l0 = 1-l1;
        const rgb& c0 = it0->second.first;
        const rgb& c1 = it1->second.first;

        return rgb( l0*c0.r + l1*c1.r, l0*c0.g + l1*c1.g, l0*c0.b + l1*c1.b, l0*c0.a + l1*c1.a);
    }

    hsv Gradient::hsvcolor(double x) const {
        if(!stops.size()) return hsv();

        stopit it0, it1;
        double l1 = findPoint(x, it0, it1);
        double l0 = 1-l1;
        const hsv& c0 = it0->second.second;
        const hsv& c1 = it1->second.second;

        return hsv( l0*c0.h + l1*c1.h, l0*c0.s + l1*c1.s, l0*c0.v + l1*c1.v, l0*c0.a + l1*c1.a);
    }

    Gradient Gradient::subGradient(double, double) const {
        Gradient G;
        if(!stops.size()) return G;
        throw; /// \todo MPM implement this!
        return G;
    }
}


