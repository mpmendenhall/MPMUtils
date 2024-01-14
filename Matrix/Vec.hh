/// @file Vec.hh Templatized fixed-length array class with mathematical operations

/*
 * Vec.hh, part of the MPMUtils package.
 * Copyright (c) 2007-2016 Michael P. Mendenhall
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

#ifndef VEC_HH
#define VEC_HH

#include <stdlib.h>
#include <iostream>
#include <array>
#include "GeomCalcUtils.hh"

using std::ostream;
using std::array;

/// Fixed-length vector arithmetic class
template<size_t N, typename T>
class Vec: public array<T,N> {
public:
    /// parent class
    typedef array<T,N> super;

    /// inherit array constructors
    using super::super;
    /// Default constructor (explicitly needed by older compilers)
    Vec(): super() { }
    /// Constructor from array
    explicit Vec(const super& a): super(a) { }
    /// Construct a basis vector with 1 in the n^th spot
    static Vec basis(size_t n) { Vec v; v[n] = 1; return v; }

    /// dot product with another vector
    template<typename U>
    T dot(const U& v) const { return ::dot(*this,v); }
    /// square magnitude \f$ v \cdot v \f$
    T mag2() const { return ::mag2(*this); }
    /// magnitude \f$ \sqrt{v\cdot v} \f$
    T mag() const { return ::mag(*this); }
    /// sum of vector elements
    T sum() const { T s = (*this)[0]; for(size_t i=1; i<N; i++) s += (*this)[i]; return s; }
    /// product of vector elements
    T prod() const { T s = (*this)[0]; for(size_t i=1; i<N; i++) s *= (*this)[i]; return s; }

    /// this vector, normalized to magnitude 1
    Vec normalized() const { return (*this)/mag(); }
    /// project out component parallel to another vector
    Vec paraProj(const Vec& v) const { return v*(dot(v)/v.mag2()); }
    /// project out component orthogonal to another vector
    Vec orthoProj(const Vec& v) const { return (*this)-paraProj(v); }

    /// unary minus operator
    const Vec operator-() const { auto v = *this; for(size_t i=0; i<N; i++) v[i] = -v[i]; return v; }

    /// inplace addition
    Vec& operator+=(const Vec& rhs) { for(size_t i=0; i<N; i++) (*this)[i] += rhs[i]; return *this; }
    /// inplace addition of a constant
    Vec& operator+=(const T& c) { for(auto& x: *this) x += c; return *this; }
    /// inplace subtraction
    Vec& operator-=(const Vec& rhs) { for(size_t i=0; i<N; i++) (*this)[i] -= rhs[i]; return *this; }
    /// inplace subtraction of a constant
    Vec& operator-=(const T& c) { for(auto& x: *this) x -= c; return *this; }

    /// inplace multiplication
    Vec& operator*=(const T& c) { for(auto& x: *this) x *= c; return *this; }
    /// inplace elementwise multiplication
    Vec& operator*=(const Vec& other) { for(size_t i=0; i<N; i++) (*this)[i] *= other[i]; return *this; }
    /// inplace division
    Vec& operator/=(const T& c) { for(auto& x: *this) x /= c; return *this; }
    /// inplace elementwise division
    Vec& operator/=(const Vec& other) { for(size_t i=0; i<N; i++) (*this)[i] /= other[i]; return *this; }

    /// addition operator
    const Vec operator+(const Vec& other) const { auto result = *this; return (result += other); }
    /// subtraction operator
    const Vec operator-(const Vec& other) const { auto result = *this; return (result -= other); }

    /// multiplication operator
    const Vec operator*(const T& c) const { auto result = *this; return (result *= c); }
    /// elementwise multiplication operator
    const Vec operator*(const Vec& other) const { auto result = *this; return (result *= other); }
    /// division operator
    const Vec operator/(const T& c) const { auto result = *this; return (result /= c); }
    /// elementwise division operator
    const Vec operator/(const Vec& other) const { auto result = *this; return (result /= other); }

    /// type conversion
    template<typename W>
    explicit operator Vec<N,W>() const {
        Vec<N,W> r;
        for(size_t i=0; i<N; i++) r[i] = W((*this)[i]);
        return r;
    }
};

/// string output representation for vectors
template<size_t N, typename T>
ostream& operator<<(ostream& o, const Vec<N,T>& v) {
    o << "<\t";
    for(size_t i=0; i<N; i++) {
        if(i) o << ",\t";
        o << v[i];
    }
    o << "\t>";
    return o;
}

// cross product of 2-vectors
//template<typename T>
//T cross( const Vec<2,T>& v1, const Vec<2,T>& v2 ) { return v1[0]*v2[1]-v1[1]*v2[0]; }

/// rotation of a 2-vector 90 degrees counterclockwise
template<typename T>
inline Vec<2,T> rhOrtho( const Vec<2,T>& v ) { return {-v[1],v[0]}; }

/// rotation of a 2-vector by given angle
template<typename T>
inline Vec<2,T> rotated(const Vec<2,T>& v, T a ) { return {v[0]*cos(a)-v[1]*sin(a), v[1]*cos(a)+v[0]*sin(a)}; }

/// orthonormal 2-vector 90 degrees counterclockwise of given 2-vector
template<typename T>
inline Vec<2,T> rhOrthoNorm( const Vec<2,T>& v ) { return Vec<2,T>(-v[1],v[0]).normalized(); }

/// vec2 from polar form specification
template<typename T>
inline Vec<2,T> polarVec(T r, T th) { return {r*cos(th), r*sin(th)}; }

#endif
