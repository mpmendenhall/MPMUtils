/* 
 * Matrix.hh, part of the MPMUtils package.
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

/// \file Matrix.hh Templatized fixed-size matrix class
#ifndef MATRIX_HH
/// Make sure this header is only loaded once
#define MATRIX_HH

#include "Vec.hh"
#include <vector>

/// A templatized, fixed size, statically allocated matrix class.
/**
 * Not particularly optimized or clever, but convenient for smallish matrices
 * or matrices of unusual special types (e.g. a matrix of circulant matrices)
 * template<unsigned int M, unsigned int N, typename T>
 */
template<unsigned int M, unsigned int N, typename T>
class Matrix {
public:
    /// constructor
    Matrix(): vv(Vec<M*N,T>()) { }
    
    /// constructor from vector
    Matrix(const Vec<M*N,T>& v): vv(v) {}
    
    /// destructor
    ~Matrix() {}
    
    /// generate a random-filled matrix
    static Matrix<M,N,T> random();
    /// generate identity matrix
    static Matrix<M,N,T> identity();
    /// generate rotation between two axes
    static Matrix<M,N,T> rotation(unsigned int a1, unsigned int a2, T th);
    
    /// const element access
    const T& operator()(unsigned int m, unsigned int n) const { assert(m<M && n<N); return vv[m+n*M]; }
    /// mutable element access
    T& operator()(unsigned int m, unsigned int n) { assert(m<M && n<N); return vv[m+n*M]; }
    /// direct access to data vector
    const Vec<M*N,T>& getData() const { return vv; }
    /// mutable vector element access
    T& operator[](unsigned int i) { return vv[i]; }
    /// const vector element access
    const T& operator[](unsigned int i) const { return vv[i]; }
    /// row vector
    Vec<N,T> row(unsigned int i) const;
    /// column vector
    Vec<M,T> col(unsigned int i) const;
    
    /// transposed copy
    Matrix<N,M,T> transposed() const;
    ///unary minus
    const Matrix<M,N,T> operator-() const;
    
    /// inplace multiplication by a constant
    void operator*=(const T& c) { vv *= c; }
    /// multiplication by a constant
    const Matrix<M,N,T> operator*(const T& c) const;
    /// inplace division by a constant
    void operator/=(const T& c) { vv /= c; }
    /// division by a constant
    const Matrix<M,N,T> operator/(const T& c) const;
    /// inplace addition of a matrix
    void operator+=(const Matrix<M,N,T>& rhs) { vv += rhs.getData(); }
    /// addition of a matrix
    const Matrix<M,N,T> operator+(const Matrix<M,N,T>& rhs) const;
    /// inplace subtraction of a matrix
    void operator-=(const Matrix<M,N,T>& rhs) { vv -= rhs.getData(); }
    /// subtraction of a matrix
    const Matrix<M,N,T> operator-(const Matrix<M,N,T>& rhs) const;
    
    /// matrix multiplication
    template<unsigned int L>
    const Matrix<M,L,T> operator*(const Matrix<N,L,T>& B) const;
    /// left-multiply a vector
    const Vec<M,T> lMultiply(const Vec<N,T>& v) const;
    /// right-multiply a vector
    const Vec<N,T> rMultiply(const Vec<M,T>& v) const;
    /// vector multiplication
    const Vec<M,T> operator*(const Vec<N,T>& v) const { return lMultiply(v); }
    
    /// inplace inverse
    const Matrix<M,N,T>& invert();
    
private:
    Vec<M*N,T> vv;
    
    /// step in inversion process
    void subinvert(unsigned int n);
};

template<unsigned int M, unsigned int N, typename T>
Matrix<M,N,T> Matrix<M,N,T>::random() { 
    Matrix<M,N,T> foo; 
    for(unsigned int i=0; i<M*N; i++)
        foo[i] = 0.1+T(rand())/T(RAND_MAX);
    return foo; 
}

template<unsigned int M, unsigned int N, typename T>
Matrix<M,N,T> Matrix<M,N,T>::identity() {
    Matrix<M,N,T> foo; 
    for(unsigned int i=0; i < std::min(M,N); i++)
        foo(i,i) = 1;
    return foo;
}

template<unsigned int M, unsigned int N, typename T>
Matrix<M,N,T> Matrix<M,N,T>::rotation(unsigned int a1, unsigned int a2, T th) {
    assert(a1 < std::min(M,N) && a2 < std::min(M,N) && a1 != a2);
    Matrix<M,N,T> foo = Matrix<M,N,T>::identity();
    foo(a1,a1) = foo(a2,a2) = cos(th);
    foo(a2,a1) = sin(th);
    foo(a1,a2) = -foo(a2,a1);
    return foo;
}

template<unsigned int M, unsigned int N, typename T>
Matrix<N,M,T> Matrix<M,N,T>::transposed() const {
    Matrix<N,M,T> foo;
    for(unsigned int r=0; r<M; r++)
        for(unsigned int c=0; c<N; c++)
            foo(c,r) = (*this)(r,c);
        return foo;
}

template<unsigned int M, unsigned int N, typename T>
Vec<N,T> Matrix<M,N,T>::row(unsigned int i) const {
    Vec<N,T> v;
    for(unsigned int j=0; j<N; j++) v[i] = (*this)(i,j);
    return v;
}

template<unsigned int M, unsigned int N, typename T>
Vec<M,T> Matrix<M,N,T>::col(unsigned int i) const {
    Vec<M,T> v;
    for(unsigned int j=0; j<M; j++) v[i] = (*this)(j,i);
    return v;
}


template<unsigned int M, unsigned int N, typename T>
const Matrix<M,N,T> Matrix<M,N,T>::operator-() const {
    Matrix<M,N,T> foo; 
    for(unsigned int i=0; i<M*N; i++)
        foo[i] = -(*this)[i];
    return foo;
}

template<unsigned int M, unsigned int N, typename T>
const Matrix<M,N,T> Matrix<M,N,T>::operator*(const T& c) const {
    Matrix<M,N,T> foo = *this;
    foo *= c;
    return foo;
}

template<unsigned int M, unsigned int N, typename T>
const Matrix<M,N,T> Matrix<M,N,T>::operator/(const T& c) const {
    Matrix<M,N,T> foo = *this;
    foo /= c;
    return foo;
}

template<unsigned int M, unsigned int N, typename T>
const Matrix<M,N,T> Matrix<M,N,T>::operator+(const Matrix<M,N,T>& rhs) const {
    Matrix<M,N,T> foo = *this;
    foo += rhs;
    return foo;
}


template<unsigned int M, unsigned int N, typename T>
const Matrix<M,N,T> Matrix<M,N,T>::operator-(const Matrix<M,N,T>& rhs) const {
    Matrix<M,N,T> foo = *this;
    foo -= rhs;
    return foo;
}

template<unsigned int M, unsigned int N, typename T>
template<unsigned int L>
const Matrix<M,L,T> Matrix<M,N,T>::operator*(const Matrix<N,L,T>& B) const {
    Matrix<M,L,T> C = Matrix<M,L,T>();
    for(unsigned int r=0; r<M; r++) {
        for(unsigned int c=0; c<L; c++) {
            C(r,c) = (*this)(r,0)*B(0,c);
            for(unsigned int i=1; i<N; i++)
                C(r,c) += (*this)(r,i)*B(i,c);
        }
    }
    return C;
}

template<unsigned int M, unsigned int N, typename T>
const Vec<M,T> Matrix<M,N,T>::lMultiply(const Vec<N,T>& v) const {
    Vec<M,T> a = Vec<M,T>();
    for(unsigned int r=0; r<M; r++)
        for(unsigned int c=0; c<N; c++)
            a[r] += (*this)(r,c)*v[c];
        return a;
}

template<unsigned int M, unsigned int N, typename T>
const Vec<N,T> Matrix<M,N,T>::rMultiply(const Vec<M,T>& v) const {
    Vec<N,T> a = Vec<N,T>();
    for(unsigned int r=0; r<M; r++)
        for(unsigned int c=0; c<N; c++)
            a[c] += v[r]*(*this)(r,c);
        return a;
}


template<unsigned int M, unsigned int N, typename T>
const Matrix<M,N,T>& Matrix<M,N,T>::invert() {
    assert(M==N);
    subinvert(0);
    return *this;
}





namespace matrix_element_inversion {
    
    template<typename T>
    inline void invert_element(T& t) { t.invert(); }
    template<>
    inline void invert_element(float& t) { t = 1.0/t; }
    template<>
    inline void invert_element(double& t) { t = 1.0/t; }
    
}

template<unsigned int M, unsigned int N, typename T>
void Matrix<M,N,T>::subinvert(unsigned int n) {
    
    // invert the first cell
    T& firstcell = (*this)(n,n);
    matrix_element_inversion::invert_element(firstcell);
    for(unsigned int i=n+1; i<M; i++)
        (*this)(n,i) *= firstcell;
    //(*this)(n,i) = firstcell*(*this)(n,i);
    
    // use to clear first column
    for(unsigned int r=n+1; r<M; r++) {
        T& m0 = (*this)(r,n);
        for(unsigned int c=n+1; c<M; c++)
            (*this)(r,c) -= (*this)(n,c)*m0;
        m0 *= -firstcell;
        //m0 = -m0*firstcell;
    }
    
    if(n==M-1)
        return;
    
    //invert the submatrix
    subinvert(n+1);
    
    // temporary space allocation
    vector<T> subvec = vector<T>(M-n-1);
    
    // first column gets multiplied by inverting submatrix
    for(unsigned int r=n+1; r<M; r++)
        subvec[r-n-1] = (*this)(r,n);
    for(unsigned int r=n+1; r<M; r++) {
        (*this)(r,n) = (*this)(r,n+1)*subvec[0];
        for(unsigned int c=n+2; c<M; c++)
            (*this)(r,n) += (*this)(r,c)*subvec[c-n-1];
    }
    
    //finish off by cleaning first row
    for(unsigned int c=n+1; c<M; c++)
        subvec[c-n-1] = (*this)(n,c);
    for(unsigned int c=n; c<M; c++) {
        if(c>n)
            (*this)(n,c) = -(*this)(n+1,c) * subvec[0];
        else
            (*this)(n,c) -= (*this)(n+1,c) * subvec[0];
        for(unsigned int r=n+2; r<M; r++)
            (*this)(n,c) -= (*this)(r,c) * subvec[r-n-1];
    }
    
}



/// string output representation for matrix
template<unsigned int M, unsigned int N, typename T>
ostream& operator<<(ostream& o, const Matrix<M,N,T>& A) {
    for(unsigned int r=0; r<M; r++) {
        o << "| ";
        for(unsigned int c=0; c<N; c++)
            o << A(r,c) << " ";
        o << "|\n";
    }
    return o;
}

#endif
