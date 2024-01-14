/// @file Averager.hh Simple statistics accumulation
// -- Michael P. Mendenhall, 2021

#ifndef AVERAGER_HH
#define AVERAGER_HH

#include <cmath>
#include <stdio.h>

/// Weighted average with numerically-stable variance tracking
template<typename value_t = double, typename weight_t = double>
struct Averager {
    /// Default constructor
    Averager() { }

    /// Add weighted item
    void add(value_t x, weight_t w) {
        if(!w) return;

        auto wx = w*x;

        if(!sw) {
            sw = w;
            swx = wx;
            return;
        }

        double u = sw*wx - w*swx;
        sw2S += (w*w*sw2S + u*u)/(sw * w);
        sw += w;
        swx += wx;
    }

    /// Add with unity weight
    void add(value_t x) {
        if(!sw) {
            ++sw;
            swx = x;
            return;
        }

        double u = sw*x - swx;
        sw2S += (sw2S + u*u)/sw;
        ++sw;
        swx += x;
    }

    /// add with unity weight
    Averager& operator+=(value_t x) { add(x); return *this; }

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

    /// rescale weights, preserving value and variance (but scaling effective sqrt(N))
    void wscale(weight_t c) { sw *= c; swx *= c; sw2S *= c*c; }

    /// unary minus
    const Averager operator-() const { return Averager(sw, -swx, sw2S); }
    /// inplace subtraction
    Averager& operator-=(const Averager& rhs) { *this += -rhs; return *this; }
    /// subtraction
    const Averager operator-(const Averager& rhs) const { auto p = *this; p -= rhs; return p; }

    /// scalar multiplication of mean value, spread
    Averager& operator*=(value_t c) { swx *= c; sw2S *= c*c; return *this; }
    /// scalar multiplication of mean value, spread
    const Averager operator*(value_t c) const { auto p = *this; p *= c; return p; }

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
    /// Constructor with contents
    Averager(weight_t _sw, value_t _swx, value_t _sw2s): sw(_sw), swx(_swx), sw2S(_sw2s) { }

    weight_t sw{};  ///< sum of weights
    value_t swx{};  ///< weighted sum w*x
    value_t sw2S{}; ///< weighted variance (sw)^2 sigma^2
};

#endif
