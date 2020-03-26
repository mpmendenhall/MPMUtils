/// \file Quaternion.hh Quaternion operations
// -- Michael P. Mendenhall, 2019

#ifndef QUATERNION_HH
#define QUATERNION_HH

#include "Abstract.hh"
#include <array>

/// Quaternion arithmetic over arithmetic ring R
template<typename R = double>
class Quaternion: public std::array<R,4> {
public:
    /// Convenience parent-class typedef
    typedef std::array<R,4> super;
    /// Default (0) Constructor
    Quaternion(): super{} { }
    /// Constructor
    Quaternion(const R& a, const R& b, const R& c, const R& d): super{a,b,c,d} { }

    /// check if zero
    explicit operator bool() const { return (*this)[0] || (*this)[1] || (*this)[2] || (*this)[3]; }
    /// equality
    bool operator==(const Quaternion& Q) const { return (super&)*this == Q; }
    /// inequality
    bool operator!=(const Quaternion& Q) const { return !(*this == Q); }
    /// mag^2 norm
    R mag2() const { auto& X = *this; return X[0]*X[0] + X[1]*X[1] + X[2]*X[2] + X[3]*X[3]; }

    /// unary minus
    const Quaternion operator-() const { return {-(*this)[0], -(*this)[1], -(*this)[2], -(*this)[3]}; }
    /// inplace addition
    Quaternion& operator+=(const Quaternion& r) {
        auto it = r.begin();
        for(auto& x: *this) x += *(it++);
        return *this;
    }
    /// addition
    const Quaternion operator+(const Quaternion& Q) const { auto c = *this; return c += Q; }
    /// inplace subtraction
    Quaternion& operator-=(const Quaternion& Q) { return *this += -Q; }
    /// subtraction
    const Quaternion operator-(const Quaternion& Q) const { return *this + -Q; }

    /// invert this = 1/this
    void invert() {
        auto n = mag2();
        (super&)*this = {(*this)[0]/n, -(*this)[1]/n, -(*this)[2]/n, -(*this)[3]/n};
    }
    /// inverse 1/this
    Quaternion inverse() const { auto i = *this; i.invert(); return i; }

    /// inplace multiplication by Quaternion: P *= Q -> PQ
    Quaternion& operator*=(const Quaternion& Q) {
        auto& P = *this;
        return *this = { P[0]*Q[0] - P[1]*Q[1] - P[2]*Q[2] - P[3]*Q[3],
                         P[0]*Q[1] + P[1]*Q[0] + P[2]*Q[3] - P[3]*Q[2],
                         P[0]*Q[2] - P[1]*Q[3] + P[2]*Q[0] + P[3]*Q[1],
                         P[0]*Q[3] + P[1]*Q[2] - P[2]*Q[1] + P[3]*Q[0] };
    }
    /// Scalar multiplication
    Quaternion& operator*=(const R& c) { for(auto& x: *this) x *= c; return *this; }
    /// out-of-place multiplication
    const Quaternion operator*(const Quaternion& Q) const { auto c = *this; return c *= Q; }

    /// inplace division
    Quaternion& operator/=(const Quaternion& Q) { return (*this) *= Q.inverse(); }
    /// out-of-place division
    const Quaternion operator/(const Quaternion& Q) const { auto q = *this; return q /= Q; }
};

/// output representation for rational fraction
template<class R>
std::ostream& operator<<(std::ostream& o, const Quaternion<R>& q) {
    return o << "[ " << q[0] << ", " << q[1] << ", " << q[2] << ", " << q[3] << " ]";
}

#endif
