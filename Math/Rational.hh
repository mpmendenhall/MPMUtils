/// \file Rational.hh Rational numbers as lists of prime factors

#ifndef RATIONAL_HH
#define RATIONAL_HH

#include "Abstract.hh"
#include "Eratosthenes.hh"

class Rational: protected SGVec_t<> {
public:
    /// Default constructor 0
    Rational(): positive(true) { this->push_back({0,1}); }
    /// Construct from integer
    Rational(int n);
    /// Default constructor from numerator, denominator
    Rational(int n, unsigned int d);
    /// Constructor from sorted factors list
    Rational(const PrimeSieve::factors_t& f);

    /// check if 0
    operator bool() const { return  !this->size() || (*this)[0].first; }

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


    /// inplace addition
    Rational& operator+=(const Rational& r);
    /// addition
    const Rational operator+(const Rational& R) const { auto c = *this; c += R; return c; }
    /// inplace subtraction
    Rational& operator-=(const Rational& R) { return *this += -R; }
    /// subtraction
    const Rational operator-(const Rational& R) const { return *this + -R; }

    bool positive; ///< sign
};


/// output representation for rational fraction
std::ostream& operator<<(std::ostream& o, const Rational& r);

#endif
