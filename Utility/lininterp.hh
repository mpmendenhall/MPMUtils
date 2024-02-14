/// @file lininterp.hh Utilities for linear interpolation
// Michael P. Mendenhall, LLNL 2021

#ifndef LININTERP_HH
#define LININTERP_HH

#include <algorithm>
#include <stddef.h> // for size_t

/// fractional index position in array
struct fracindex_t {
    /// default constructor
    fracindex_t() { }
    /// constructor from fractional value
    explicit fracindex_t(double x): i(x > 0? x : 0), j(x >= 0? x-i : -1) { }
    /// constructor from components
    fracindex_t(size_t _i, double _j): i(_i), j(_j) { }
    /// convert back to double
    operator double() const { return double(i) + j; }

    size_t i = 0;   ///< upper-bound index; 0 <= i <= size()
    double j = 0;   ///< fractional part 0 <= j <= 1 unless out-of-bounds (= -1 or 2)
};

/// locate point in sorted range
template<class ARRAY, typename VAL>
fracindex_t locate(const VAL& x, const ARRAY& A) {
    fracindex_t i;
    i.i = std::lower_bound(A.begin(), A.end(), x) - A.begin();

    if(!i.i) {
        if(x < A.at(0)) i.j = -1;
        return i;
    }

    if(i.i == A.size()) {
        i.j = 2;
        return i;
    }

    auto dy = A[i.i] - A[i.i-1];
    if(dy) i.j = (x - A[i.i-1])/dy;
    return i;
}

/// linearly interpolate to fractional index, clamped to ends outside range
template<class ARRAY>
double lininterp(const ARRAY& A, fracindex_t i) {
    if(!i.i) return A.at(0);
    if(i.i >= A.size()) return A.back();
    return A[i.i - 1]*(1-i.j) + A[i.i]*i.j;
}

#endif
