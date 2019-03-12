/// \file ModularField.hh Modular Integers field

#ifndef MODULARFIELD_HH
#define MODULARFIELD_HH

#include "Abstract.hh"
#include <stdexcept>

/// Modular Integers field with N elements
template<size_t N>
class ModularField {
public:
    /// Default constructor to 0
    ModularField() { }
    /// Default constructor to 0
    ModularField(int n): i((n<0? N*(-n/N + 1) + n : n)%N) { }

    /// check if 0
    operator bool() const { return  i; }
    /// cast to int
    operator int() const { return i; }

    /// comparison
    bool operator<(const ModularField<N>& Z) const { return i < Z.i; }
    /// equality
    bool operator==(const ModularField<N>& Z) const { return i == Z.i; }
    /// inequality
    bool operator!=(const ModularField<N>& Z) const { return i != Z.i; }

    /// unary minus
    const ModularField<N> operator-() const { return -i; }

    /// invert contents
    //Rational& invert() { if(!*this) throw std::range_error("1/0 is bad!"); for(auto& kv: *this) { kv.second = -kv.second; } return *this; }

    /// inplace multiplication
    ModularField<N>& operator*=(const ModularField<N>& Z) { i = (i*Z.i)%N; return *this; }
    /// multiplication
    const ModularField<N> operator*(const ModularField<N>& Z) const { auto c = *this; return c *= Z; }
    /// inplace division
    //ModularField<N>& operator/=(ModularField<N> R) { return (*this) *= R.invert(); }
    /// division
    //const ModularField<N> operator/(ModularField<N> R) const { return *this * R.invert(); }

    /// inplace addition
    ModularField<N>& operator+=(const ModularField<N>& Z) { i = (i+Z.i)%N; return *this; }
    /// addition
    const ModularField<N> operator+(const ModularField<N>& Z) const { auto c = *this; return c += Z; }
    /// inplace subtraction
    ModularField<N>& operator-=(const ModularField<N>& Z) { return *this += -Z; }
    /// subtraction
    const ModularField<N> operator-(const ModularField<N>& Z) const { return *this + -Z; }

protected:
    int i = 0;  ///< internal representation in [0,N)
};

/// output representation for rational fraction
template<size_t N>
std::ostream& operator<<(std::ostream& o, const ModularField<N>& Z) { o << int(Z); return o; }

#endif
