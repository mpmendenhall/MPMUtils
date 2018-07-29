/// \file BBox.hh Templatized D-dimensional bounding box
// This file was produced under the employ of the United States Government,
// and is consequently in the PUBLIC DOMAIN, free from all provisions of
// US Copyright Law (per USC Title 17, Section 105).
//
// -- Michael P. Mendenhall, 2015

#ifndef BBOX_HH
#define BBOX_HH

#include <limits>
#include <array>

/// Templatized D-dimensional bounding box
template<size_t D, typename T>
class BBox {
public:
    /// coordinate type
    typedef std::array<T,D> coord_t;

    /// expand to include point
    void expand(const T* x) {
        for(size_t i=0; i<D; i++) {
            if(x[i] < lo[i]) lo[i] = x[i];
            if(x[i] > hi[i]) hi[i] = x[i];
        }
    }

    /// expand to include point (from array)
    void expand(const coord_t& a) { expand(a.data()); }

    /// offset by vector
    void offset(const T* x) {
        for(size_t i=0; i<D; i++) {
            lo[i] += x[i];
            hi[i] += x[i];
        }
    }

    /// offset by vector
    void offset(const coord_t& a) { offset(a.data()); }

    /// expand to include BBox
    void operator+=(const BBox& B) {
        if(B.isNull()) return;
        expand(B.lo);
        expand(B.hi);
    }

    /// expand by margin on all sides
    void expand(T x) {
        for(auto& v: lo) v -= x;
        for(auto& v: hi) v += x;
    }

    /// check if point in half-open [lo, hi) interior
    bool inside(const T* x) const {
        for(size_t i=0; i<D; i++)
            if(!(lo[i] <= x[i] && x[i] < hi[i])) return false;
        return true;
    }

    /// width along axis
    T dl(size_t i) const { return hi[i] - lo[i]; }
    /// local coordinate along axis, 0->lo and 1->hi
    T pos(T x, size_t i) const { return lo[i] + x*dl(i); }

    coord_t lo; /// lower bounds
    coord_t hi; /// upper bounds

    /// create BBox with "null" bounds
    static BBox nullBox() {
        BBox<D,double> b;
        for(auto& v: b.lo) v = std::numeric_limits<T>::max();
        for(auto& v: b.hi) v = -std::numeric_limits<T>::max();
        return b;
    }

    bool isNull() const {
        for(size_t i=0; i<D; i++) if(hi[i] < lo[i]) return true;
        return false;
    }

    /// equality check
    bool operator==(const BBox& B) const { return lo == B.lo && hi == B.hi; }
};

#endif
