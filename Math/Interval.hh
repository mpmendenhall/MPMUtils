/// \file Interval.hh 1D interval
// Michael P. Mendenhall, 2020

#ifndef INTERVAL_HH
#define INTERVAL_HH

#include <limits>

/// 1-dimensional interval
template<typename T = double>
class Interval {
public:
    /// value type
    typedef T value_t;

    value_t lo = std::numeric_limits<T>::max();     /// lower bound
    value_t hi{-std::numeric_limits<T>::max()};     /// upper bound

    /// expand to include point
    void expand(T x) {
        if(x < lo) lo = x;
        if(x > hi) hi = x;
    }
    /// expand to include Interval
    void operator+=(const Interval& B) {
        if(B.isNull()) return;
        expand(B.lo);
        expand(B.hi);
    }
    /// offset by vector
    void offset(T dx) { lo += dx; hi += dx; }

    /// check if point in half-open [lo, hi) interior
    bool inside(T x) const { return lo <= x && x < hi; }
    /// width along axis
    T dl() const { return hi - lo; }
    /// local coordinate along axis, 0->lo and 1->hi
    T pos(T x) const { return lo + x*dl(); }
    /// check if null interval
    bool isNull() const { return !(hi >= lo); }
};

#endif
