/// \file Rational.hh Rational numbers as lists of prime factors

#ifndef RATIONAL_HH
#define RATIONAL_HH

#include "Abstract.hh"
#include "Eratosthenes.hh"
#include <stdexcept>

/// Rational numbers as sorted list of prime factors; implements operations for field
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

    /// comparison
    bool operator<(const Rational& R) const;
    /// equality
    bool operator==(const Rational& R) const { return components() == R.components(); }
    /// inequality
    bool operator!=(const Rational& R) const { return !this->operator==(R); }

    /// unary minus
    const Rational operator-() const { auto r = *this; r.positive = !positive; return r; }

    /// invert contents
    Rational& invert() { if(!*this) throw std::range_error("1/0 is bad!"); for(auto& kv: *this) { kv.second = -kv.second; } return *this; }

    /// inplace multiplication
    Rational& operator*=(const Rational& R);
    /// multiplication
    const Rational operator*(const Rational& R) const { auto c = *this; return c *= R; }
    /// inplace division
    Rational& operator/=(Rational R) { return (*this) *= R.invert(); }
    /// division
    const Rational operator/(Rational R) const { return *this * R.invert(); }
    /// raise to integer power
    const Rational pow(int i) const;


    /// inplace addition
    Rational& operator+=(const Rational& r);
    /// addition
    const Rational operator+(const Rational& R) const { auto c = *this; return c += R; }
    /// inplace subtraction
    Rational& operator-=(const Rational& R) { return *this += -R; }
    /// subtraction
    const Rational operator-(const Rational& R) const { return *this + -R; }

    bool positive = true; ///< sign
};

/// absolute value of rational number
inline Rational rabs(Rational r) { r.positive = true; return r; }

/// output representation for rational fraction
std::ostream& operator<<(std::ostream& o, const Rational& r);

/// Euclid's algorithm for relatively prime numbers, returning c,d: c*p = d*q + 1
pair<int,int> EuclidRelPrime(int p, int q);

/// Goal: expand p/q -> a + b/p1 + c/p2 + ..., primes p1,p2,...

#endif
