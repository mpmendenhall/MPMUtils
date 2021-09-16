/// \file Averager.hh Simple statistics accumulation
// -- Michael P. Mendenhall, 2021

#ifndef AVERAGER_HH
#define AVERAGER_HH

#include <cmath>
#include <stdio.h>

/// Weighted average with numerically-stable variance tracking
template<typename value_t = double, typename weight_t = double>
struct Averager {
    /// Add weighted item
    void add(value_t v, weight_t w = 1) {
        if(!sw) {
            sw = w;
            swx = v*w;
            return;
        }
        if(!w) return;

        auto vw = v*w;
        double u = sw*vw - w*swx;
        sw2S += (w*w*sw2S + u*u)/(sw * w);
        sw += w;
        swx += vw;
    }

    /// add with unity weight
    Averager& operator+=(value_t v) { add(v); return *this; }

    /// add averager
    Averager& operator+=(const Averager& a) {
        if(!a.sw) return *this;
        if(!sw) return (*this = a);

        double u = sw*a.swx - a.sw*swx;
        sw2S += a.sw2S + (sw*sw*a.sw2S + a.sw*a.sw*sw2S + u*u)/(sw * a.sw);
        sw += a.sw;
        swx += a.swx;
        return *this;
    }

    /// total weight
    weight_t weight() const { return sw; }
    /// get average
    value_t average() const { return swx/sw; }
    /// get average
    explicit operator value_t() const { return average(); }
    /// get mean square deviation
    value_t variance() const { return sw2S/(sw * sw); }
    /// RMS variation
    value_t sigma() const { return sqrt(sw2S)/sw; }
    /// sqrt(N)-weighted uncertainty
    value_t uncert() const { return sqrt(variance()/sw); }
    /// uncertainty squared
    value_t uncert2() const { return variance()/sw; }

    /// print info to stdout
    void display() const { printf("mu = %g, sigma = %g (w = %g)\n", average(), sigma(), weight()); }

protected:
    weight_t sw{};  ///< sum of weights
    value_t swx{};  ///< weighted sum w*x
    value_t sw2S{}; ///< weighted variance (sw)^2 sigma^2
};

#endif
