/// \file Rational.hh Rational numbers as lists of prime factors

#ifndef RATIONAL_HH
#define RATIONAL_HH

#include "Abstract.hh"
#include "Eratosthenes.hh"

/// Rational numbers as lists of prime factors; implements operations for field
class Rational: protected SGVec_t<> {
public:
    /// for iterating through contents
    using SGVec_t<>::begin;
    /// for iterating through contents
    using SGVec_t<>::end;
    /// for map-style construction
    typedef map<gen_t, num_t> fmap_t;

    /// Default constructor to 0
    Rational() { emplace_back(0,1); }
    /// Constructor from numerator, denominator
    Rational(int n, int d);
    /// constructor from numerator
    Rational(int n) { *this = Rational(n,1); }
    /// Constructor from sorted factors list
    Rational(const PrimeSieve::factors_t& f);
    /// From factor : power list
    Rational(const fmap_t& m, bool pos = true);

    /// check if 0 (explictly constructed as 0^1); empty contents => 1
    operator bool() const { return  size() != 1 || (*this)[0].first; }

    /// numerator, denominator pair
    pair<int,int> components() const;

    /// integer evaluation
    operator int() const { auto c = components(); return c.first / c.second; }
    /// floating point evaluation
    operator double() const { auto c = components(); return double(c.first) / c.second; }

    /// comparison
    bool operator<(const Rational& R) const;

    /// unary minus
    const Rational operator-() const { auto r = *this; r.positive = !positive; return r; }

    /// invert contents
    Rational& invert() { for(auto& kv: *this) { kv.second = -kv.second; } return *this; }

    /// inplace multiplication
    Rational& operator*=(const Rational& R);
    /// multiplication
    const Rational operator*(const Rational& R) const { auto c = *this; c *= R; return c; }
    /// inplace division
    Rational& operator/=(Rational R) { return (*this) *= R.invert(); }
    /// division
    const Rational operator/(Rational R) const { return *this * R.invert(); }
    /// raise to integer power
    const Rational pow(int i) const;


    /// inplace addition
    Rational& operator+=(const Rational& r);
    /// addition
    const Rational operator+(const Rational& R) const { auto c = *this; c += R; return c; }
    /// inplace subtraction
    Rational& operator-=(const Rational& R) { return *this += -R; }
    /// subtraction
    const Rational operator-(const Rational& R) const { return *this + -R; }

    bool positive = true; ///< sign
};


/// output representation for rational fraction
std::ostream& operator<<(std::ostream& o, const Rational& r);

#endif
