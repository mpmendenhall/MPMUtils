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

/// \file Vec.hh Templatized fixed-length array class with mathematical operations
#ifndef VEC_HH
/// Make sure this header is only loaded once
#define VEC_HH

#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <array>
#include <vector>

using std::ostream;
using std::vector;
using std::array;

/// Fixed-length vector arithmetic class
template<size_t N, typename T>
class Vec: public array<T,N> {
public:
    
    /// Default constructor for zero-filled vector
    Vec() { }
    /// Constructor from std::array
    Vec(const array<T,N>& a): array<T,N>(a) { }
    /// Construct a basis vector with 1 in the n^th spot
    static Vec<N,T> basis(size_t n) { Vec<N,T> v = Vec<N,T>(); v[n] = 1; return v; }
    
    /// dot product with another vector
    T dot(const Vec<N,T>& v) const { T s = (*this)[0]*v[0]; for(size_t i=1; i<N; i++) s+=(*this)[i]*v[i]; return s; }
    /// square magnitude \f$ v \cdot v \f$
    T mag2() const { return dot(*this); }
    /// magnitude \f$ \sqrt{v\cdot v} \f$
    T mag() const { return sqrt(mag2()); }
    /// sum of vector elements
    T sum() const { T s = (*this)[0]; for(size_t i=1; i<N; i++) s += (*this)[i]; return s; }
    /// product of vector elements
    T prod() const { T s = (*this)[0]; for(size_t i=1; i<N; i++) s *= (*this)[i]; return s; }
    
    /// this vector, normalized to magnitude 1
    Vec<N,T> normalized() const { return (*this)/mag(); }
    /// project out component parallel to another vector
    Vec<N,T> paraProj(const Vec<N,T>& v) const { return v*(dot(v)/v.mag2()); }
    /// project out component orthogonal to another vector
    Vec<N,T> orthoProj(const Vec<N,T>& v) const { return (*this)-paraProj(v); }
    /// angle with another vector
    T angle(const Vec<N,T> v) const { return acos(dot(v)/sqrt(mag2()*v.mag2())); }
    
    /// unary minus operator
    const Vec<N,T> operator-() const { auto v = *this; for(size_t i=0; i<N; i++) v[i] = -v[i]; return v; }
    
    /// inplace addition
    Vec<N,T>& operator+=(const Vec<N,T>& rhs) { for(size_t i=0; i<N; i++) (*this)[i] += rhs[i]; return *this; }
    /// inplace addition of a constant
    Vec<N,T>& operator+=(const T& c) { for(auto& x: *this) x += c; return *this; }
    /// inplace subtraction
    Vec<N,T>& operator-=(const Vec<N,T>& rhs) { for(size_t i=0; i<N; i++) (*this)[i] -= rhs[i]; return *this; }
    /// inplace subtraction of a constant
    Vec<N,T>& operator-=(const T& c) { for(auto& x: *this) x -= c; return *this; }
    
    /// inplace multiplication
    Vec<N,T>& operator*=(const T& c) { for(auto& x: *this) x *= c; return *this; }
    /// inplace elementwise multiplication
    Vec<N,T>& operator*=(const Vec<N,T>& other) { for(size_t i=0; i<N; i++) (*this)[i] *= other[i]; return *this; }
    /// inplace division
    Vec<N,T>& operator/=(const T& c) { for(auto& x: *this) x /= c; return *this; }
    /// inplace elementwise division
    Vec<N,T>& operator/=(const Vec<N,T>& other) { for(size_t i=0; i<N; i++) (*this)[i] /= other[i]; return *this; }
    
    /// addition operator
    const Vec<N,T> operator+(const Vec<N,T>& other) const { auto result = *this; result += other; return result; }
    /// subtraction operator
    const Vec<N,T> operator-(const Vec<N,T>& other) const { auto result = *this; result -= other; return result; }
    
    /// multiplication operator
    const Vec<N,T> operator*(const T& c) const { auto result = *this; result *= c; return result; }
    /// elementwise multiplication operator
    const Vec<N,T> operator*(const Vec<N,T>& other) const { auto result = *this; result *= other; return result; }
    /// division operator
    const Vec<N,T> operator/(const T& c) const { auto result = *this; result /= c; return result; }
    /// elementwise division operator
    const Vec<N,T> operator/(const Vec<N,T>& other) const { auto result = *this; result /= other; return result; }
        
    /// write in binray form to a file
    void writeBinary(ostream& o) const { o.write((char*)this->data(),N*sizeof(T)); }
    /// read a Vec from a file
    static Vec<N,T> readBinary(std::istream& s) { Vec<N,T> v; v.loadBinaryData(s); return v; }
    /// read in binary form from a file
    Vec<N,T>& loadBinaryData(std::istream& s) { s.read((char*)this->data(),N*sizeof(T)); return *this; }
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

/// cross product of 2-vectors
template<typename T>
T cross( const Vec<2,T>& v1, const Vec<2,T>& v2 ) { return v1[0]*v2[1]-v1[1]*v2[0]; }

/// cross product of 3-vectors
template<typename T>
Vec<3,T> cross( const Vec<3,T>& a, const Vec<3,T>& b ) {
    return Vec<3,T>(a[1]*b[2]-b[1]*a[2], a[2]*b[0]-b[2]*a[0], a[0]*b[1]-b[0]*a[1]);
}

/// rotation of a 2-vector 90 degrees counterclockwise
template<typename T>
Vec<2,T> rhOrtho( const Vec<2,T>& v ) {
    return Vec<2,T>(-v[1],v[0]); 
}

/// rotation of a 2-vector by given angle
template<typename T>
Vec<2,T> rotated(const Vec<2,T>& v, T a ) {
    return Vec<2,T>( v[0]*cos(a)-v[1]*sin(a), v[1]*cos(a)+v[0]*sin(a) );
}

/// orthonormal 2-vector 90 degrees counterclockwise of given 2-vector
template<typename T>
Vec<2,T> rhOrthoNorm( const Vec<2,T>& v ) { 
    return Vec<2,T>(-v[1],v[0]).normalized();
}

/// atan2() angle of a 2-vector
template<typename T>
T angle( const Vec<2,T>& v ) { 
    return atan2(v[1],v[0]);
}

/// vec2 from polar form specification
template<typename T>
Vec<2,T> polarVec(T r, T th) {
    return Vec<2,T>(r*cos(th),r*sin(th));
}

/// Vec to vector<double>
template<size_t N, typename T>
vector<double> vec2doublevec(const Vec<N,T>& v) {
    vector<double> dv(N);
    for(size_t i=0; i<N; i++) dv[i] = (double)v[i];
    return dv;
}


#endif
    