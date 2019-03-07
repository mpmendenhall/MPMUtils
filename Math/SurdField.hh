/// \file SurdField.hh Field of sums of square roots of rational numbers

#ifndef SURDFIELD_HH
#define SURDFIELD_HH

#include "Rational.hh"
#include <set>
#include <cmath>

/// Square root of prime factors
class PrimeRoot_t: public std::set<int> {
public:
    /// multiply square roots
    std::pair<int, PrimeRoot_t> mul(const PrimeRoot_t& b) const;
    /// squared value
    int square() const { int i = 1; for(auto x: *this) i *= x; return i; }
    /// double value
    operator double() const { return sqrt(square()); }
};

/// Sums of square roots of rational numbers, with field operations
class SurdSum: protected std::map<PrimeRoot_t, Rational> {
public:
    /// Default 0 constructor
    SurdSum() { }
    /// Constructor from integer
    SurdSum(const int i) { *this = SurdSum(Rational(i)); }
    /// Constructor from rational
    SurdSum(const Rational& R) { if(R) this->emplace(PrimeRoot_t(), R); }

    /// Square-root of rational
    static SurdSum sqrt(const Rational& R);

    /// check if 0
    operator bool() const { return size(); }
    /// double value
    operator double() const { double s = 0; for(auto& kv: *this) s += double(kv.first)*double(kv.second); return s; }

    /// unary minus
    const SurdSum operator-() const { auto r = *this; for(auto& kv: r) kv.second = -kv.second; return r; }
    /// invert this = 1/this
    void invert();
    /// inverse 1/this
    SurdSum inverse() const { auto i = *this; i.invert(); return i; }

    /// inplace multiplication by SurdSum
    SurdSum& operator*=(const SurdSum& R);
    /// inplace multiplication by rational
    SurdSum& operator*=(const Rational& R) { if(!R) clear(); else for(auto& kv: *this) kv.second *= R; return *this; }
    /// inplace multiplication by integer
    SurdSum& operator*=(int i) { if(!i) clear(); else for(auto& kv: *this) kv.second *= i; return *this; }

    /// out-of-place multiplication
    template<class T>
    const SurdSum operator*(const T& R) const { auto c = *this; c *= R; return c; }

    /// inplace division
    SurdSum& operator/=(const SurdSum& R) { return (*this) *= R.inverse(); }
    /// division
    SurdSum operator/(const SurdSum& R) const { auto q = *this; return q /= R; }

    /// inplace addition
    SurdSum& operator+=(const SurdSum& r);
    /// addition
    template<class T>
    const SurdSum operator+(const T& R) const { auto c = *this; c += R; return c; }
    /// inplace subtraction
    SurdSum& operator-=(const SurdSum& R) { return *this += -R; }
    /// subtraction
    template<class T>
    const SurdSum operator-(const T& R) const { return *this + -R; }


    /// Separate out terms containing prime root factor: separate(3, 2+sqrt(15)) -> (sqrt(5), 2)
    pair<SurdSum,SurdSum> separateRoot(int i) const;

    friend std::ostream& operator<<(std::ostream& o, const SurdSum& r);
};

/// output representation for rational fraction
std::ostream& operator<<(std::ostream& o, const SurdSum& r);

#endif
