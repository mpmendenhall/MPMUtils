/// \file PhiField.hh Field of a + b*phi (a,b rational; phi = golden ratio)
// Michael P. Mendenhall, 2019

#ifndef PHIFIELD_HH
#define PHIFIELD_HH

#include "SurdField.hh"

/// field of a+b*phi, phi = (1+sqrt(5))/2
class PhiField {
public:
    /// Default 0 constructor
    PhiField(): a({}), b({}) { }
    /// Constructor
    PhiField(Rational aa, Rational bb): a(aa), b(bb) { }

    /// convert to SurdSum
    operator SurdSum() const { return SurdSum(a + b/2) + SurdSum(b/2)*SurdSum::sqrt(5); }
    /// convert to double
    explicit operator double() const { return double(a) + double(b)*0.5*(1+sqrt(5.)); }

    /// equality with rational (also picks up int)
    bool operator==(const Rational& R) const { return a == R && !b; }
    /// inequality
    template<typename T>
    bool operator!=(const T& x) const { return !(*this == x); }
    /// comparison
    bool operator<(const PhiField& P) const;
    /// comparison with rational (also picks up int)
    bool operator<(const Rational& R) const;
    /// check if nonzero
    explicit operator bool() const { return a || b; }

    /// unary minus
    const PhiField operator-() const { return {-a,-b}; }
    /// inplace addition
    PhiField& operator+=(const PhiField& r) { a += r.a; b += r.b; return *this; }
    /// inplace addition of Rational (also picks up int)
    PhiField& operator+=(const Rational& r) { a += r; return *this; }
    /// addition (automatic type detection)
    template<class T>
    const PhiField operator+(const T& R) const { auto c = *this; return c += R; }
    /// inplace subtraction (automatic type detection)
    template<class T>
    PhiField& operator-=(const T& R) { return *this += -R; }
    /// subtraction (automatic type detection)
    template<class T>
    const PhiField operator-(const T& R) const { return *this + -R; }

    /// invert this = 1/this
    void invert() {
        auto x = 2*a + b;
        if(!x) { b = 4/(5*b); a = -b/2; }
        else { x = x*x-5*b; *this = {4*(a+b)/x, -4*b/x}; }
    }
    /// inverse 1/this
    PhiField inverse() const { auto i = *this; i.invert(); return i; }

    /// inplace multiplication by PhiField
    PhiField& operator*=(const PhiField& P) { return (*this = {a*P.a + b*P.b, a*P.b + b*P.a + b*P.b}); }
    /// inplace multiplication by Rational (also picks up int)
    PhiField& operator*=(const Rational& R) { a *= R; b *= R; return *this; }
    /// out-of-place multiplication (automatic type detection)
    template<class T>
    const PhiField operator*(const T& R) const { auto c = *this; return c *= R; }

    /// inplace division
    PhiField& operator/=(const PhiField& R) { return (*this) *= R.inverse(); }
    /// inplace rational division, also picks up int
    PhiField& operator/=(const Rational& R) { a /= R; b /= R; return *this; }
    /// out-of-place division (automatic type detection)
    template<class T>
    const PhiField operator/(const T& R) const { auto q = *this; return q /= R; }

    /// number '1'
    static inline PhiField one() { return {1,0}; }
    /// number 'phi'
    static inline PhiField phi() { return {0,1}; }

    Rational a; /// coefficient of '1'
    Rational b; /// coefficient of 'phi'
};

/// output representation for PhiField
std::ostream& operator<<(std::ostream& o, const PhiField& r);

#endif
