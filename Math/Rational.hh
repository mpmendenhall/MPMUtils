/// \file Rational.hh Rational numbers as lists of prime factors

#ifndef RATIONAL_HH
#define RATIONAL_HH

#include "Abstract.hh"
#include "Eratosthenes.hh"

class Rational: protected SGVec_t<> {
public:
    /// Default constructor from numerator, denominator
    Rational(int n = 0, unsigned int d = 1);
    /// Constructor from sorted factors list
    Rational(const PrimeSieve::factors_t& f);

    /// invert contents
    void invert() { for(auto& kv: *this) kv.second = -kv.second; }

    /// check if 0
    bool operator!() const { return this->size() && !(*this)[0].first; }

    /// inplace multiplication
    Rational& operator*=(const Rational& R);
    /// multiplication
    const Rational operator*(const Rational& R) { auto c = *this; c *= R; return c; }

    /// numerator, denominator pair
    pair<int,int> components() const;

    /// integer evaluation
    operator int() const { auto c = components(); return c.first / c.second; }

    /// inplace addition
    Rational& operator+=(const Rational& r);
    /// addition
    const Rational operator+(const Rational& R) const { auto c = *this; c += R; return c; }
};


/// output representation for rational fraction
std::ostream& operator<<(std::ostream& o, const Rational& r);

#endif
