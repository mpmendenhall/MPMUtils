/// \file Rational.hh Rational numbers as lists of prime factors

#ifndef RATIONAL_HH
#define RATIONAL_HH

#include "Abstract.hh"
#include "Eratosthenes.hh"
#include <stdexcept>

/// Rational numbers as sorted list of prime factors; implements operations for field
//  empty set = 1; special case 0 represented by + 0^1
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
    Rational(int n, int d = 1);
    /// From factor : power list
    Rational(const fmap_t& m, bool pos = true);
    /// to double
    double val() const { auto nd = components(); return double(nd.first)/nd.second; }
    /// check if nonzero; DANGER: unwanted conversion to int!
    //operator bool() const { return  !isZero(); }

    /// (signed) numerator, (unsigned) denominator pair
    pair<int,int> components() const;

    /// comparison
    bool operator<(const Rational& R) const;
    /// equality
    bool operator==(const Rational& R) const { return (SGVec_t<>&)(*this) == R && positive == R.positive; }
    /// check if zero
    bool isZero() const { return size() == 1 && !(*this)[0].first; }
    /// check if +/-1
    bool isUnit() const { return !size(); }
    /// check if integral (no negative powers)
    bool isIntegral() const { for(auto& kv: *this) if(kv.second < 0) return false; return true; }
    /// optimized check for equality with integer
    bool operator==(int i) const;
    /// inequality
    template<typename T>
    bool operator!=(const T& x) const { return !(*this == x); }

    /// unary minus
    const Rational operator-() const { auto r = *this; r.positive = (isZero()? true : !positive); return r; }

    /// invert contents
    Rational& invert();

    /// inplace multiplication
    Rational& operator*=(const Rational& R);
    /// inplace int multiplication: needed if accidental cast to int possible
    //Rational& operator*=(int i) { return (*this) *= Rational(i); }
    /// out-of-place multiplication
    template<typename T>
    const Rational operator*(const T& x) const { auto c = *this; return c *= x; }

    /// inplace division
    Rational& operator/=(Rational R) { return (*this) *= R.invert(); }
    /// inplace integer division: needed if accidental cast to int possible
    //Rational& operator/=(int i) { return (*this) *= Rational(i).invert(); }
    /// out-of-place division
    template<typename T>
    const Rational operator/(T x) const { auto c = *this; return c /= x; }
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

protected:
    /// Constructor from sorted factors list != 1
    Rational(const PrimeSieve::factors_t& f);
};

/// absolute value of rational number
inline Rational rabs(Rational r) { r.positive = true; return r; }

/// output representation for rational fraction
std::ostream& operator<<(std::ostream& o, const Rational& r);

#endif
