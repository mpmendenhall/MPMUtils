/* 
 * CoordTransform.hh, part of the MPMUtils package.
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

/// \file CoordTransform.hh Templatized rotation/translation coordinate transform
#ifndef COORDTRANSFORM_HH
/// Make sure this header is only loaded once
#define COORDTRANSFORM_HH

#include "Matrix.hh"

/// A templatized rotation+translation coordinate tranform
/**
 * Coordinate transform with a vector offset and orthogonal
 * transformation, applied rotation first, then translation.
 * These follow algebra, letting [T] denote translation by vector T,
 * and R be a rotation matrix, acting on right-hand-side operands:
 * [T] R = R [R^-1 T]
 * ([T] R)^-1 = R^-1 [-T] = [-R^-1 T] R^-1
 * [T'] [T] R = [T' + T]
 * [R'] [T] R = [R' T] R' R
 * [T'] R' [T] R = [T' + R' T] R' R
 * where, for orthogonal R, R^-1 = transpose(R) --- though this algebra also works for general matrices M
 */
template<unsigned int N, typename T>
class CoordTransform {
public:
    /// Constructor
    CoordTransform(): R(Matrix<N,N,T>::identity()) { }

    /// Inverse
    CoordTransform<N,T> inverse() const { return CoordTransform(R.rMultiply(-dx), R.transposed()); }
    /// Inverse, general non-orthogonal-matrix case
    CoordTransform<N,T> inverseGeneral() const { auto M = R.inverse(); return CoordTransform(M*(-dx), M); }
    /// Left-multiplied by (orthogonal) transform matrix
    CoordTransform<N,T>& operator*=(const Matrix<N,N,T>& M) { dx = M * dx; R = M * R; return *this; }
    /// Compose additional translation
    CoordTransform<N,T>& operator+=(const Vec<N,T>& v) { dx += v; return *this; }
    /// Left-multiplied composition this = other * this
    CoordTransform<N,T>& operator*=(const CoordTransform<N,T>& other) { return ((*this *= other.R) += other.dx); }
    
    /// Apply to position
    Vec<N,T> operator*(const Vec<N,T>& rhs) const { return dx + (R*rhs); }
    /// Apply rotation to vector
    Vec<N,T> rotate(const Vec<N,T>& rhs) const { return R*rhs; }

private:
    /// Constructor from components
    CoordTransform(const Vec<N,T>& dx0, const Matrix<N,N,T> R0): dx(dx0), R(R0) { }

    Vec<N,T> dx;        ///< offset
    Matrix<N,N,T> R;    ///< rotation
};

#endif
