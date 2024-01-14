/// @file Interval.hh 1D interval
// Michael P. Mendenhall, 2020

#ifndef INTERVAL_HH
#define INTERVAL_HH

#include <limits>

/// An interval
template<typename T = double>
class Interval {
public:
    /// value type
    typedef T value_t;

    value_t lo; ///< lower end of interval
    value_t hi; ///< upper end of interval

    /// Default null-interval constructor
    Interval():
    lo(std::numeric_limits<T>::max()),
    hi(-std::numeric_limits<T>::max()) { }

    /// Constructor with range
    Interval(T a, T b): lo(a), hi(b) { }

    /// check if null interval
    bool isNull() const { return !(hi >= lo); }
    /// check if point in half-open [lo, hi) interior
    bool inside(T x) const { return lo <= x && x() < hi; }
    /// interval length
    T dl() const { return isNull()? T{} : hi - lo; }
    /// local coordinate along axis, 0->lo and 1->hi
    double pos(double x) const { return lo + x*dl(); }

    /// ordering comparison
    bool operator<(const Interval& r) const { return lo < r.lo || (lo == r.lo && hi < r.hi); }

    /// intersection of intervals
    const Interval& operator&=(Interval b) {
        if(hi <= b.lo || lo >= b.hi) *this = {};
        else *this = {std::max(lo, b.lo), std::min(hi, b.hi)};
        return *this;
    }
    /// intersection of intervals
    Interval operator&(Interval b) const { return b &= *this; }
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
    /// offset interval
    void offset(T dx) { lo += dx; hi += dx; }
};

#endif
