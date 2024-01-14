/// @file SurdField.hh Field of sums of square roots of rational numbers
// -- Michael P. Mendenhall, 2019

#ifndef SURDFIELD_HH
#define SURDFIELD_HH

#include "Rational.hh"
#include <set>

/// Square root of prime factors; constructor defaults to 1
class PrimeRoot_t: public std::set<int> {
public:
    /// multiply square roots
    std::pair<int, PrimeRoot_t> mul(const PrimeRoot_t& b) const;
    /// squared value
    int square() const { int i = 1; for(auto x: *this) i *= x; return i; }
    /// double value
    explicit operator double() const { return sqrt(square()); }
};

/// Sums of square roots of rational numbers, with field operations
class SurdSum: public std::map<PrimeRoot_t, Rational> {
public:
    /// Constructor from rational; defaults to 0. Automatically converts from int.
    SurdSum(const Rational& R = {}) { if(R) emplace(PrimeRoot_t(), R); }
    /// Square-root of rational (or auto from int) --- imaginary allowed!
    static SurdSum sqrt(const Rational& R);

    /// check if nonzero
    explicit operator bool() const { return size(); }
    /// double value
    explicit operator double() const { double s = 0; for(auto& kv: *this) s += double(kv.first)*double(kv.second); return s; }
    /// comparison
    bool operator<(const SurdSum& S) const { return double(*this-S) < 0.0; }
    /// equality with rational (also picks up int)
    bool operator==(const Rational& R) const { return (!R && !*this) || (size() == 1 && !begin()->first.size() && begin()->second == R); }
    /// inequality
    template<typename T>
    bool operator!=(const T& x) const { return !(*this == x); }

    /// unary minus
    const SurdSum operator-() const { auto r = *this; for(auto& kv: r) kv.second = -kv.second; return r; }
    /// inplace addition
    SurdSum& operator+=(const SurdSum& r);
    /// inplace addition of Rational (also picks up int)
    SurdSum& operator+=(const Rational& r);
    /// addition (automatic type detection)
    template<class T>
    const SurdSum operator+(const T& R) const { auto c = *this; return c += R; }
    /// inplace subtraction (automatic type detection)
    template<class T>
    SurdSum& operator-=(const T& R) { return *this += -R; }
    /// subtraction (automatic type detection)
    template<class T>
    const SurdSum operator-(const T& R) const { return *this + -R; }

    /// invert this = 1/this
    void invert();
    /// inverse 1/this
    SurdSum inverse() const { auto i = *this; i.invert(); return i; }

    /// inplace multiplication by SurdSum
    SurdSum& operator*=(const SurdSum& R);
    /// inplace multiplication by Rational (also picks up int)
    SurdSum& operator*=(const Rational& R) { if(!R) clear(); else for(auto& kv: *this) kv.second *= R; return *this; }
    /// out-of-place multiplication (automatic type detection)
    template<class T>
    const SurdSum operator*(const T& R) const { auto c = *this; return c *= R; }

    /// inplace division
    SurdSum& operator/=(const SurdSum& R) { return (*this) *= R.inverse(); }
    /// inplace rational division, also picks up int
    SurdSum& operator/=(const Rational& R) { for(auto& kv: *this) kv.second /= R; return *this; }
    /// out-of-place division (automatic type detection)
    template<class T>
    const SurdSum operator/(const T& R) const { auto q = *this; return q /= R; }

    /// Separate out terms containing prime root factor: separate(3, 2+sqrt(15)) -> (sqrt(5), 2)
    pair<SurdSum,SurdSum> separateRoot(int i) const;

    friend std::ostream& operator<<(std::ostream& o, const SurdSum& r);
};

/// output representation for rational fraction
std::ostream& operator<<(std::ostream& o, const SurdSum& r);

#endif
