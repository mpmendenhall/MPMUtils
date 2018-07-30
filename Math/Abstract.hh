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

/////////////////////////////////////////////////////////////////////////////////////////
// semigroup: set, operator *: a,b in S => a*b in s
//
// monoid: semigroup with identity element 1 in S, 1*a = a*1 = a for all a in S
//
// group: monoid with inverses; for all a in S, exists a^-1 such that a a^-1 = 1 = a^-1 a
//
// ring: set, operator *, operator +
//      Abelian group under + => associative, commutative, has identity 0 and inverse -a
//      monoid under * (has identity 1); * is distributive over +, not necessarily commutative
//
// field: ring plus commutative over +, * inverses except 0
//
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
#include <cmath>

//template<typename A>
//using array_contents_t = typename std::tuple_element<0,A>::type;

template<typename T>
using array_contents_t = typename std::remove_reference<decltype(std::declval<T&>()[0])>::type;


template<typename M>
using map_key_t = typename std::remove_reference<decltype(std::declval<M&>().begin()->first)>::type;

template<typename M>
using map_value_t = typename std::remove_reference<decltype(std::declval<M&>().begin()->second)>::type;

/// Formal abstract polynomial, with +/- and * operations
// R is a ring with operators +,*, inverse -, default constructor '0'
// E is a semigroup with operator + for exponent symbols x^i, x^k, x^(i+k)
template<typename R, typename E>
class AbstractPolynomial: public std::map<E,R> {
public:
    /// monomials semigroup member type
    typedef E monomial_t;
    /// coefficients ring member type
    typedef R coeff_t;

    /// default constructor
    using std::map<E,R>::map;

    //////////////////////////////////////
    // core "required" polyomial functions

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

    /////////////////////////////////
    // optional variants as-supported

    /// transform as sum of terms
    template<class X>
    auto transform(const X& x) const -> decltype(x(this->begin()->first,this->begin()->second)) {
        decltype(x(this->begin()->first,this->begin()->second)) P;
        for(auto& kv: *this) P += x(kv.first,kv.second);
        return P;
    }
};

// transform patterns on P = sum(c*x^i)
// x^i -> k * y^j => P -> sum (c*k) y^j term-by-term or drop
// OR terms are polynomials that mix, x^i -> P'(k,y)
// P -> sum c*P'(k,y) = P''(sum c*k, y), new type c*k

/// output representation
template<typename R, typename E>
std::ostream& operator<<(std::ostream& o, const AbstractPolynomial<R,E>& p) {
    o << "< ";
    for(auto& kv: p) o << kv.second << " * " << kv.first << " ";
    o << ">";
    return o;
}

/// Wrapper for iterable fixed-size array into semigroup under +
template<typename A>
class ArraySemigroup: public A {
public:
    /// convenience typedef for array contents
    typedef array_contents_t<A> x_t;
    /// an appropriate coordinate type
    template<typename T>
    using coord_t = std::array<T, std::tuple_size<A>::value>;

    /// default constructor
    ArraySemigroup(): A() { }
    /// constructor from array
    ArraySemigroup(const A& v): A(v) { }

    /// inplace addition
    ArraySemigroup& operator+=(const ArraySemigroup& rhs) { size_t i=0; for(auto& e: *this) { e += rhs[i++]; } return *this; }
    /// addition
    const ArraySemigroup operator+(const ArraySemigroup& rhs) const { auto s = *this; s += rhs; return s; }

    /// total contents 1-norm
    x_t order() const { x_t o = 0; for(auto e: *this) o += fabs(e); return o; }
    /// const access at index i
    x_t get(size_t i) const { return this->at(i); }

    /// evaluate at given point, treating as exponentiation
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

/// Wrapper for map-type object as semigroup under +
template<typename M>
class MapSemigroup: public M {
public:
    /// convenience typedef for map contents
    typedef map_value_t<M> x_t;
    /// convenience typedef for map key
    typedef map_key_t<M> k_t;
    /// an appropriate coordinate type
    template<typename T>
    using coord_t = std::map<k_t, T>;


    /// default constructor
    MapSemigroup(): M() { }
    /// constructor from array
    MapSemigroup(const M& m): M(m) { }

    /// inplace addition
    MapSemigroup& operator+=(const MapSemigroup& rhs) {
        for(auto& kv: rhs) (*this)[kv.first] += kv.second;
        return *this;
    }
    /// addition
    const MapSemigroup operator+(const MapSemigroup& rhs) const { auto s = *this; s += rhs; return s; }

    /// total contents 1-norm
    x_t order() const { x_t o = 0; for(auto& kv: *this) o += fabs(kv.second); return o; }
    /// const access at index i
    x_t get(k_t i) const {
        auto it = this->find(i);
        return it == this->end? x_t() : it->second;
    }

    /// evaluate at given point, treating as exponentiation
    template<typename coord>
    auto operator()(const coord& m) const -> decltype(m.begin()->second) {
        decltype(m.begin()->second) s = 1;
        for(auto& kv: *this) {
            auto it = m.find(kv.first);
            if(it == m.end()) continue;
            auto e = it->first;
            while(e--) s *= it->second;
        }
        return s;
    }
};

/// convenience typedef for N-dimensional (unsigned int) exponents vector
template<long unsigned int N, typename T = unsigned int>
using ExpVec_t = ArraySemigroup<std::array<T,N>>;

/// output representation for Semigroup<Vec>
template<typename Vec>
std::ostream& operator<<(std::ostream& o, const ArraySemigroup<Vec>& v) {
    o << "( ";
    for(auto& e: v) o << e << " ";
    o << ")";
    return o;
}

#endif
