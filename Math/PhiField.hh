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
    operator SurdSum() const { return SurdSum(a + b/Rational(2)) + SurdSum(b/Rational(2))*SurdSum::sqrt(5); }

    /// check if nonzero
    operator bool() const { return a || b; }
    /// equality
    bool operator==(const PhiField& S) const { return a == S.a && b == S.b; }
    /// inequality
    bool operator!=(const PhiField& S) const { return !(*this == S); }
    /// comparison
    bool operator<(const SurdSum& S) const { return SurdSum(*this) < S; }

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
        auto x = a*Rational(2) + b;
        if(!x) { b = Rational(4,5)/b; a = -b/Rational(2); }
        else { x = x*x-Rational(5)*b; *this = {(a+b)*Rational(4)/x, b*Rational(-4)/x}; }
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
