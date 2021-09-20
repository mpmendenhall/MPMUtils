/// \file Matrix.hh Templatized fixed-size matrix class

/*
 * Matrix.hh, part of the MPMUtils package.
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

#ifndef MATRIX_HH
#define MATRIX_HH

#include "Vec.hh"
#include <vector>
using std::vector;
#include <cassert>
#include "iffy_constexpr.hh"

/// Handle min in a==b case without tripping -Wduplicated-branches warnings
template <class T>
constexpr T constexpr_min(T a, T b) { return a < b ? a : b; }

/// A templatized, fixed size, statically allocated matrix class, stored in rows order.
/**
 * Not particularly optimized or clever, but convenient for smallish matrices
 * or matrices of symbolic special types (e.g. a matrix of circulant matrices)
 */
template<size_t M, size_t N, typename T>
class Matrix: public Vec<M*N,T> {
protected:
    /// convenience typedef
    typedef Vec<M*N,T> super;
public:
    /// number of rows
    static constexpr size_t nRows = M;
    /// number of columns
    static constexpr size_t nCols = N;
    /// min(M,N)
    static constexpr size_t nDiag = constexpr_min(nRows,nCols);

    /// inherit constructors from Vec
    using super::super;

    /// generate a random-filled matrix
    static Matrix random();
    /// generate identity matrix
    static _constexpr Matrix identity();
    /// generate rotation between two axes
    static constexpr Matrix rotation(size_t a1, size_t a2, T th);

    /// const element access
    const T& operator()(size_t m, size_t n) const { assert(m<M && n<N); return (*this)[m*N+n]; }
    /// mutable element access
    T& operator()(size_t m, size_t n) { assert(m<M && n<N); return (*this)[m*N+n]; }
    /// row vector
    Vec<N,T> row(size_t i) const;
    /// column vector
    Vec<M,T> col(size_t i) const;

    /// transposed copy
    Matrix<N,M,T> transposed() const;
    ///unary minus
    const Matrix operator-() const;

    /// inplace multiplication by a constant
    Matrix& operator*=(const T& c) { super::operator*=(c); return *this; }
    /// multiplication by a constant
    const Matrix operator*(const T& c) const { auto foo = *this; return (foo *= c); }
    /// inplace division by a constant
    Matrix& operator/=(const T& c) { super::operator/=(c); return *this; }
    /// division by a constant
    const Matrix operator/(const T& c) const { auto foo = *this; return (foo /= c); }
    /// inplace addition of a matrix
    Matrix& operator+=(const Matrix& rhs) { super::operator+=(rhs); return *this; }
    /// addition of a matrix
    const Matrix operator+(const Matrix& rhs) const { auto foo = *this; return (foo += rhs); }
    /// inplace subtraction of a matrix
    Matrix& operator-=(const Matrix& rhs) { super::operator-=(rhs); return *this; }
    /// subtraction of a matrix
    const Matrix operator-(const Matrix& rhs) const { auto foo = *this; return (foo -= rhs); }

    /// matrix multiplication
    template<size_t L>
    const Matrix<M,L,T> operator*(const Matrix<N,L,T>& B) const;
    /// left-multiply a vector
    const Vec<M,T> lMultiply(const Vec<N,T>& v) const;
    /// right-multiply a vector
    const Vec<N,T> rMultiply(const Vec<M,T>& v) const;
    /// vector left-multiplication
    const Vec<M,T> operator*(const Vec<N,T>& v) const { return lMultiply(v); }

    /// inplace inverse
    const Matrix& invert();
    /// out-of-place inverse
    Matrix inverse() const { auto i = *this; return i.invert(); };

    /// trace
    const T trace() const { T t = (*this)(0,0); for(size_t i = 1; i<nDiag; ++i) t += (*this)(i,i); return t; }

// fix conflict with GSL(?) macro
#ifdef minor
#undef minor
#endif

    /// Matrix minor (remove row i, column j)
    const Matrix<M-1,N-1,T> minor(size_t i, size_t j) const {
        static_assert(M && N, "Asking for minor of 0-element matrix is impolite!");

        Matrix<M-1, N-1, T> m;
        size_t rr = 0;
        for(size_t r=0; r<M; r++) {
            if(r==i) continue;
            size_t cc = 0;
            for(size_t c=0; c<N; c++) {
                if(c==j) continue;
                m(rr,cc++) = (*this)(r,c);
            }
            ++rr;
        }
        return m;
    }

    /// type conversion
    template<typename W>
    explicit operator Matrix<M,N,W>() const {
        Matrix<M,N,W> r;
        for(size_t i=0; i<nRows*nCols; i++) r[i] = W((*this)[i]);
        return r;
    }
};

/// right-multiply vector * matrix
template<size_t M, size_t N, typename T>
const Vec<N,T> operator*(const Vec<M,T>& v, const Matrix<M,N,T>& X) { return X.rMultiply(v); }

/// unnormalized axis of 3D rotation
template<typename T>
Vec<3,T> R3axis(const Matrix<3,3,T>& M) {
    auto t = M.trace();

    if(t == 3) return {}; // M is identity matrix; return null axis

    if(t == -1) { // special case: rotations by pi
        for(auto i: {0,1,2}) {
            if(M(i,i) == 1) // special case: rotation around a coordinate axis
                return Vec<3,T>{{ (M(0,0)+1)/2, (M(1,1)+1)/2, (M(2,2)+1)/2 }};
        }
        return Vec<3,T>{{ M(0,1)*M(0,2)*4, M(1,0)*M(1,2)*4, M(2,0)*M(2,1)*4 }};
    }

    return Vec<3,T>{{M(2,1)-M(1,2), M(0,2)-M(2,0), M(1,0)-M(0,1)}};
}

/// string output representation for matrix
template<size_t M, size_t N, typename T>
ostream& operator<<(ostream& o, const Matrix<M,N,T>& A) {
    for(size_t r=0; r<M; r++) {
        o << "| ";
        for(size_t c=0; c<N; c++)
            o << A(r,c) << " ";
        o << "|\n";
    }
    return o;
}


/////////////////////////////////////
/////////////////////////////////////
/////////////////////////////////////

/// unit/identity value for type
template<typename T>
inline T unit() { return 1; }

template<size_t M, size_t N, typename T>
Matrix<M,N,T> Matrix<M,N,T>::random() {
    Matrix foo;
    for(auto& x: foo) x = 0.1+T(rand())/T(RAND_MAX);
    return foo;
}

template<size_t M, size_t N, typename T>
_constexpr Matrix<M,N,T> Matrix<M,N,T>::identity() {
    Matrix foo{};
    for(size_t i=0; i < nDiag; i++) foo(i,i) = unit<T>();
    return foo;
}

template<size_t M, size_t N, typename T>
Matrix<N,M,T> Matrix<M,N,T>::transposed() const {
    Matrix<N,M,T> foo;
    for(size_t r=0; r<M; r++)
        for(size_t c=0; c<N; c++)
            foo(c,r) = (*this)(r,c);
    return foo;
}

template<size_t M, size_t N, typename T>
Vec<N,T> Matrix<M,N,T>::row(size_t i) const {
    Vec<N,T> v;
    for(size_t j=0; j<N; j++) v[i] = (*this)(i,j);
    return v;
}

template<size_t M, size_t N, typename T>
Vec<M,T> Matrix<M,N,T>::col(size_t i) const {
    Vec<M,T> v;
    for(size_t j=0; j<M; j++) v[j] = (*this)(j,i);
    return v;
}

template<size_t M, size_t N, typename T>
template<size_t L>
const Matrix<M,L,T> Matrix<M,N,T>::operator*(const Matrix<N,L,T>& B) const {
    Matrix<M,L,T> C;
    for(size_t r=0; r<M; r++) {
        for(size_t c=0; c<L; c++) {
            C(r,c) = (*this)(r,0)*B(0,c);
            for(size_t i=1; i<N; i++)
                C(r,c) += (*this)(r,i)*B(i,c);
        }
    }
    return C;
}

template<size_t M, size_t N, typename T>
const Vec<M,T> Matrix<M,N,T>::lMultiply(const Vec<N,T>& v) const {
    Vec<M,T> a{};
    for(size_t r=0; r<M; r++) {
        for(size_t c=0; c<N; c++) a[r] += (*this)(r,c)*v[c];
    }
    return a;
}

template<size_t M, size_t N, typename T>
const Vec<N,T> Matrix<M,N,T>::rMultiply(const Vec<M,T>& v) const {
    Vec<N,T> a{};
    for(size_t r=0; r<M; r++) {
        for(size_t c=0; c<N; c++) a[c] += v[r]*(*this)(r,c);
    }
    return a;
}

///////////////////////////
// Square matrix operations
///////////////////////////

/// Compare magnitudes |a| < |b| --- override for fancier types
template<typename T>
bool abs_lt(const T& a, const T& b) { return (a<0? -a:a) < (b<0? -b:b); }

/// LUP decomposition of A -> PA = LU, where L=1 on diagonal
template<size_t N, typename T>
class LUPDecomp: protected Matrix<N,N,T> {
public:
    static_assert(N, "Please avoid zero-dimensional matrices.");

    /// parent class
    typedef Matrix<N,N,T> super;

    /// Default constructor
    LUPDecomp() { }

    /// Constructor, from matrix to decompose
    explicit LUPDecomp(const super& A): super(A) {
        // initial unit permutation
        size_t i = 0;
        for(auto& x: P) x = i++;

        for(i=0; i<N; ++i) {
            // determine maximum-magnitude row in current column
            T mm{};
            auto imax = i;
            for(size_t k = i; k < N; k++) if(abs_lt(mm,(*this)(k,i))) { mm = (*this)(k,i); imax = k; }
            if(!i) cMin = mm;
            else if(abs_lt(mm, cMin)) cMin = mm;
            // singular?
            if(!mm) { nP = -1; return; }

            // pivoting as needed
            if(imax != i) {
                std::swap(P[i], P[imax]);
                for(size_t j=0; j<N; j++) std::swap((*this)(i,j), (*this)(imax,j));
                ++nP;
            }

            for(size_t j = i+1; j<N; ++j) {
                (*this)(j,i) /= (*this)(i,i);
                for(size_t k = i+1; k<N; k++) (*this)(j,k) -= (*this)(j,i) * (*this)(i,k);
            }
        }
    }

    /// Solve A*x = b for x
    template<typename V>
    V solve(const V& b) const {
        if(isSingular()) throw std::runtime_error("Matrix is singular!");

        V x{};
        for(size_t i = 0; i < N; ++i) {
            x[i] = b[P[i]];
            for(size_t k = 0; k < i; ++k) x[i] -= (*this)(i,k) * x[k];
        }

        for (int i = N-1; i >= 0; --i) {
            for(size_t k = i + 1; k < N; ++k) x[i] -= (*this)(i,k) * x[k];
            x[i] /= (*this)(i,i);
        }
        return x;
    }

    /// Check if matrix is (near-)singular, according to initial construction tolerance
    bool isSingular() const { return nP < 0; }

    /// Determinant of A
    const T det() const {
        if(isSingular()) return T{};
        auto d = (*this)(0,0);
        for(size_t i=1; i<N; i++) d *= (*this)(i,i);
        return (nP % 2)? -d : d;
    }

    /// Fill Ai with inverse of A
    void inverse(super& Ai) const {
        if(isSingular()) throw std::runtime_error("Matrix is singular!");

        for(size_t j = 0; j < N; ++j) {
            for(size_t i = 0; i < N; ++i) {
                Ai(i,j) = (P[i]==(int)j)? unit<T>() : T{};
                for(size_t k = 0; k < i; ++k) Ai(i,j) -= (*this)(i,k) * Ai(k,j);
            }

            for(int i = N - 1; i >= 0; --i) {
                for(size_t k = i + 1; k < N; ++k) Ai(i,j) -= (*this)(i,k) * Ai(k,j);
                Ai(i,j) /= (*this)(i,i);
            }
        }
    }

    /// extract L
    super L() const {
        auto _L =  super::identity();
        for(size_t i=0; i<N; ++i)
            for(size_t j=0; j<i; ++j)
                _L(i,j) = (*this)(i,j);
        return _L;
    }

    /// extract U
    super U() const {
        super _U{};
        for(size_t i=0; i<N; ++i)
            for(size_t j=i; j<N; ++j)
                _U(i,j) = (*this)(i,j);
        return _U;
    }

protected:
    array<int,N> P;     ///< pivots permutation
    int nP = 0;         ///< number of permutation swaps
    T cMin{};           ///< smallest row-division value encountered
};

/// Determinant
template<size_t M, typename T>
const T det(const Matrix<M,M,T>& X) {
    LUPDecomp<M,T> LU(X);
    return LU.det();
}

template<size_t M, size_t N, typename T>
const Matrix<M,N,T>& Matrix<M,N,T>::invert() {
    static_assert(M==N, "Inverse for square matrix only!");
    LUPDecomp<N,T> LU(*this);
    LU.inverse(*this);
    return *this;
}

#endif
