/// \file Rational.hh Rational numbers as lists of prime factors

#ifndef RATIONAL_HH
#define RATIONAL_HH

#include "Abstract.hh"
#include "Eratosthenes.hh"
#include <stdexcept>

typedef int64_t _Rational_int_t;

/// Rational numbers as sorted list of prime factors; implements operations for field
//  empty set = 1; special case 0 represented by + 0^1
class Rational: protected SGVec_t<_Rational_int_t, int> {
public:
    /// integer values
    typedef _Rational_int_t int_t;
    /// parent type
    typedef SGVec_t<int_t,int> super;

    /// for iterating through contents
    using super::begin;
    /// for iterating through contents
    using super::end;
    /// for map-style construction
    typedef map<gen_t, num_t> fmap_t;

    /// Default constructor to 0
    Rational() { emplace_back(0,1); }
    /// Constructor from numerator, denominator
    Rational(int_t n, int_t d = 1);
    /// From factor : power list
    Rational(const fmap_t& m, bool pos = true);
    /// to double
    explicit operator double() const { auto nd = components(); return double(nd.first)/nd.second; }
    /// check if nonzero
    explicit operator bool() const { return  !isZero(); }
    /// check if > 0
    bool posdef() const { return positive && !isZero(); }
    /// check if < 0
    bool negdef() const { return !positive && !isZero(); }

    /// (signed) numerator, (unsigned) denominator pair
    pair<int_t,int_t> components() const;

    /// comparison
    bool operator<(const Rational& R) const;
    /// equality
    bool operator==(const Rational& R) const { return (super&)(*this) == R && positive == R.positive; }
    /// optimized check for equality with integer
    bool operator==(int_t i) const;
    /// inequality
    template<typename T>
    bool operator!=(const T& x) const { return !(*this == x); }
    /// check if zero
    bool isZero() const { return size() == 1 && !(*this)[0].first; }
    /// check if +/-1
    bool isUnit() const { return !size(); }
    /// check if integral (no negative powers)
    bool isIntegral() const { for(auto& kv: *this) if(kv.second < 0) return false; return true; }

    /// unary minus
    const Rational operator-() const { auto r = *this; r.positive = (isZero()? true : !positive); return r; }
    /// invert contents
    Rational& invert();
    /// get inverse
    const Rational inverse() const { auto R = *this; return R.invert(); }

    /// inplace addition
    Rational& operator+=(const Rational& r);
    /// out-of-place addition
    const Rational operator+(const Rational& R) const { auto c = *this; return c += R; }

    /// inplace subtraction
    Rational& operator-=(const Rational& R) { return *this += -R; }
    /// out-of-place subtraction
    const Rational operator-(const Rational& R) const { return *this + -R; }

    /// inplace multiplication
    Rational& operator*=(const Rational& R);
    /// out-of-place multiplication
    const Rational operator*(const Rational& x) const { auto c = *this; return c *= x; }

    /// inplace division
    Rational& operator/=(Rational R) { return (*this) *= R.invert(); }
    /// out-of-place division
    const Rational operator/(const Rational& x) const { auto c = *this; return c /= x; }

    /// raise to integer power
    const Rational pow(int i) const;

    bool positive = true; ///< sign

protected:
    /// Constructor from sorted factors list != 1
    Rational(const PrimeSieve::factors_t& f);
};

/// convenience "opposite order" addition
inline const Rational operator+(Rational::int_t i, const Rational& R) { return R+i; }
/// convenience "opposite order" subtraction
inline const Rational operator-(Rational::int_t i, const Rational& R) { return -R+i; }
/// convenience "opposite order" multiplication
inline const Rational operator*(Rational::int_t i, const Rational& R) { return R*i; }
/// convenience "opposite order" division
inline const Rational operator/(Rational::int_t i, Rational R) { return R.invert()*i; }
/// convenience "opposite order" comparison
inline const Rational operator<(Rational::int_t i, Rational R) { return !(R==i || R < i); }
/// absolute value of rational number
inline const Rational rabs(Rational r) { r.positive = true; return r; }

/// output representation for rational fraction
std::ostream& operator<<(std::ostream& o, const Rational& r);

#endif
