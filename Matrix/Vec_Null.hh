/* 
 * Vec_Null.hh, part of the MPMUtils package.
 * Copyright (c) 2007-2014 Michael P. Mendenhall
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

/// \file Vec_Null.hh Special case for 0-element vectors
#ifndef VEC_NULL_HH
/// Make sure this header is only loaded once
#define VEC_NULL_HH

#include "Vec.hh"
#include <cassert>

/// Special 0 case
template<typename T>
class Vec<0,T>: public array<T,0> {
public:
    
    /// default constructor for zero vector
    Vec() { }
    
    /// dot product with another vector
    T dot(const Vec<0,T>) const { return T(); }
    /// square magnitude \f$ v \cdot v \f$
    T mag2() const { return T(); }
    /// magnitude \f$ \sqrt{v\cdot v} \f$
    T mag() const { return T(); }
    /// sum of vector elements
    T sum() const { return T(); }
    /// product of vector elements
    T prod() const { return T(); }
    
    /// this vector, normalized to magnitude 1
    Vec<0,T> normalized() const { return Vec<0,T>(); }
    /// project out component parallel to another vector
    Vec<0,T> paraProj(const Vec<0,T>) const { return Vec<0,T>(); }
    /// project out component orthogonal to another vector
    Vec<0,T> orthoProj(const Vec<0,T>) const { return Vec<0,T>(); }
    /// angle with another vector
    T angle(const Vec<0,T>) const { return T(); }
    
    /// mutable element access operator
    T& operator[](size_t) { assert(false); }
    /// immutable element access operator
    T operator[](size_t) const { assert(false); }
    
    /// unary minus operator
    const Vec<0,T> operator-() const { return Vec<0,T>(); }
    
    /// inplace addition
    Vec<0,T>& operator+=(const Vec<0,T>&) { return *this; }
    /// inplace subtraction
    Vec<0,T>& operator-=(const Vec<0,T>&) { return *this; }
    
    /// inplace multiplication
    Vec<0,T>& operator*=(T) { return *this; }
    /// inplace elementwise multiplication
    Vec<0,T>& operator*=(const Vec<0,T>&) { return *this; }
    /// inplace division
    Vec<0,T>& operator/=(T) { return *this; }
    /// inplace elementwise division
    Vec<0,T>& operator/=(const Vec<0,T>&) { return *this; }
    
    /// addition operator
    const Vec<0,T> operator+(const Vec<0,T>&) const { return Vec<0,T>(); }
    /// subtraction operator
    const Vec<0,T> operator-(const Vec<0,T>&) const { return Vec<0,T>(); }
    
    /// multiplication operator
    const Vec<0,T> operator*(T) const { return Vec<0,T>(); }
    /// elementwise multiplication operator
    const Vec<0,T> operator*(const Vec<0,T>&) const { return Vec<0,T>(); }
    /// division operator
    const Vec<0,T> operator/(T) const { return Vec<0,T>(); }
    /// elementwise division operator
    const Vec<0,T> operator/(const Vec<0,T>&) const { return Vec<0,T>(); }
};
    
#endif
    