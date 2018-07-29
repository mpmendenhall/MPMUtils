/// \file "Abstract.hh" Top-level abstract mathematical structures
/*
 * Abstract.hh, part of the MPMUtils package.
 * Copyright (c) 2007-2018 Michael P. Mendenhall
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef ABSTRACT_HH
#define ABSTRACT_HH

// semigroup: set, operator *: a,b in S => a*b in s
// monoid: semigroup with identity element 1 in S, 1*a = a*1 = a for all a in S
// group: monoid with inverses; for all a in S, exists a^-1 such that a a^-1 = 1 = a^-1 a
// ring: set, operator *, operator +
//      Abelian group under + => associative, commutative, has identity 0 and inverse -a
//      monoid under * (has identity 1); * is distributive over +, not necessarily commutative
// field: ring, add commutative over +, * inverses except 0
// vector/linear space: module over a field

// Left R-Module M:
// Abelian group (M,+)
// R is a Ring, operator * and ~
// operator x: r x m -> M
// r x (m+n) = r x m + r x n
// (r~s) x m = r x m + s x m
// (r*s) x m = r x (s x m)
// 1_R x m = m
// Right R-Module: similar with m x r -> M

#include <map>
#include <array>
#include <iostream>
#include <cassert>

template<typename A>
using array_contents_t = typename std::remove_reference<decltype(std::declval<A&>()[0])>::type;

/// Formal abstract polynomial
// R is a ring with operators +,*, inverse -, default constructor '0'
// E is a semigroup with operator + for exponent symbols x^i, x^k, x^(i+k)
template<typename R, typename E>
class AbstractPolynomial: public std::map<E,R> {
public:
    /// monomial term type
    typedef E monomial_t;
    /// coefficients type
    typedef R coeff_t;

    /// default constructor
    using std::map<E,R>::map;

    /// unary minus
    const AbstractPolynomial operator-() const { AbstractPolynomial P(*this); for(auto& kv: P) kv.second = -kv.second; }
    /// inplace addition
    AbstractPolynomial& operator+=(const AbstractPolynomial& rhs) { for(auto& kv: rhs) (*this)[kv.first] += kv.second; return *this; }
    /// addition
    const AbstractPolynomial operator+(const AbstractPolynomial& rhs) const { auto p = *this; p += rhs; return p; }
    /// inplace subtraction
    AbstractPolynomial& operator-=(const AbstractPolynomial& rhs) { for(auto& kv: rhs) (*this)[kv.first] -= kv.second; return *this; }
    /// subtraction
    const AbstractPolynomial operator-(const AbstractPolynomial& rhs) const { auto p = *this; p -= rhs; return p; }
    /// multiplication
    const AbstractPolynomial operator*(const AbstractPolynomial& rhs) const {
        AbstractPolynomial P;
        for(auto& kv: *this)
            for(auto& kv2: rhs)
                P[kv.first + kv2.first] += kv.second*kv.second;
        return P;
    }
    /// inplace multiplication
    AbstractPolynomial& operator*=(const AbstractPolynomial& rhs) { *this = (*this) * rhs; return *this; }
    /// scalar multiplication
    AbstractPolynomial& operator*=(const coeff_t& rhs) { for(auto& kv: *this) kv.second *= rhs; }
};

// output representation
template<typename R, typename E>
std::ostream& operator<<(std::ostream& o, const AbstractPolynomial<R,E>& p) {
    o << "< ";
    for(auto& kv: p) o << kv.second << " * " << kv.first << " ";
    o << ">";
    return o;
}

/// Wrapper for iterable vector into semigroup under +
template<typename Vec>
class Semigroup: public Vec {
public:
    /// default constructor
    Semigroup(): Vec() { }
    /// constructor from Vec
    Semigroup(const Vec& v): Vec(v) { }

    /// inplace addition
    Semigroup& operator+=(const Semigroup& rhs) { size_t i=0; for(auto& e: *this) { e += rhs[i++]; } return *this; }
    /// addition
    const Semigroup operator+(const Semigroup& rhs) const { auto s = *this; s += rhs; return s; }

    /// evaluate at given point
    template<typename coord>
    auto operator()(const coord& v) const -> array_contents_t<coord> {
        assert(v.size() == this->size());
        array_contents_t<coord> s = 1;
        unsigned int i = 0;
        for(auto e: *this) {
            while(e--) s *= v[i];
            i++;
        }
        return s;
    }
};

/// convenience typedef for N-dimensional (unsigned int) exponents vector
template<long unsigned int N, typename T = unsigned int>
using EVec = Semigroup<std::array<T,N>>;

// output representation
template<typename Vec>
std::ostream& operator<<(std::ostream& o, const Semigroup<Vec>& v) {
    o << "( ";
    for(auto& e: v) o << e << " ";
    o << ")";
    return o;
}

#endif
