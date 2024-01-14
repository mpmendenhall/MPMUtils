/*
 * VarMat.hh, part of the MPMUtils package.
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

/// @file VarMat.hh Templatized variable-size matrices with mathematical operations
#ifndef VARMAT_HH
/// Make sure this header is only loaded once
#define VARMAT_HH

#include "VarVec.hh"
#include "Matrix.hh"

/// A templatized, dynamically allocated matrix class.
/**
 * Not particularly optimized or clever, but convenient for smallish matrices
 * or matrices of unusual special types (e.g. block circulant matrices, with CMatrix entries).
 * Data internally in *column major* order, for easier LAPACK compatibility
 */
template<typename T>
class VarMat: public BinaryOutputObject {
public:
    /// constructor with prototype element
    explicit VarMat(size_t m=0, size_t n=0, const T& i = 0): M(m), N(n), vv(n*m,i) { }
    /// constructor from fixed matrix
    template<size_t MM, size_t NN>
    explicit VarMat(Matrix<MM,NN,T> A): M(MM), N(NN), vv(A.getData()) {}
    /// destructor
    ~VarMat() {}

    /// generate an "identity" matrix, using specified values on/off diagonal
    static VarMat<T> identity(size_t n) { return  VarMat<T>::identity(n,1,0); }
    /// generate an "identity" matrix, using specified values on/off diagonal
    static VarMat<T> identity(size_t n, const T& one, const T& zero);
    /// generate a random-filled VarMat
    static VarMat<T> random(size_t m, size_t n);

    /// get m rows
    size_t nRows() const { return M; }
    /// get n cols
    size_t nCols() const { return N; }
    /// get d ? rows : cols
    size_t nDim(bool rows) const { return rows ? M:N; }
    /// get total size
    size_t size() const { return vv.size(); }

    /// const element access
    const T& operator()(size_t m, size_t n) const { assert(m<M && n<N); return vv[m+n*M]; }
    /// mutable element access
    T& operator()(size_t m, size_t n) { assert(m<M && n<N); return vv[m+n*M]; }
    /// direct access to data vector
    const VarVec<T>& getData() const { return vv; }
    /// mutable vector element access
    T& operator[](size_t i) { return vv[i]; }
    /// const vector element access
    const T& operator[](size_t i) const { return vv[i]; }
    /// append column
    void appendCol(const VarVec<T>& v) { if(!nRows() && !nCols()) M=v.size(); assert(v.size() == nRows()); vv.append(v); N++; }
    /// append matrix of columns
    void appendCols(const VarMat<T>& C) { if(!nRows() && !nCols()) M=C.nRows(); assert(C.nRows() == nRows()); vv.append(C.vv); N += C.nCols(); }
    /// get row vector
    VarVec<T> getRow(size_t i) const;
    /// get column vector
    VarVec<T> getCol(size_t i) const;
    /// get sum of each column
    VarVec<T> getColSum() const;
    /// get sum of each row
    VarVec<T> getRowSum() const;
    /// get sum of squares of elements
    T getSumSquares() const { return vv.mag2(); }
    /// get determinant TODO for M>2
    T det() const { if(M!=N) return 0; if(M==1) return vv[0]; if(M==2) return vv[0]*vv[3]-vv[1]*vv[2]; assert(false); return 0; }

    /// unary minus (negated copy)
    const VarMat<T> operator-() const;
    /// transposed copy
    VarMat<T> transposed() const;
    /// inplace inverse
    const VarMat<T>& invert();
    /// inplace resize, truncating or adding default elements
    const VarMat<T>& resize(size_t m, size_t n);
    /// trace of matrix
    T trace() const;

    /// throw error if dimensions mismatches
    void checkDimensions(const VarMat& m) const { if(m.nRows() != M || m.nCols() != N) throw(DimensionMismatchError()); }

    /// inplace multiplication by a constant
    void operator*=(const T& c) { vv *= c; }
    /// multiplication by a constant
    const VarMat<T> operator*(const T& c) const;
    /// inplace division by a constant
    void operator/=(const T& c) { vv /= c; }
    /// division by a constant
    const VarMat<T> operator/(const T& c) const;
    /// inplace addition of a VarMat
    void operator+=(const VarMat<T>& rhs) { checkDimensions(rhs); vv += rhs.getData(); }
    /// addition of a VarMat
    const VarMat<T> operator+(const VarMat<T>& rhs) const;
    /// inplace subtraction of a VarMat
    void operator-=(const VarMat<T>& rhs) { checkDimensions(rhs); vv -= rhs.getData(); }
    /// subtraction of a VarMat
    const VarMat<T> operator-(const VarMat<T>& rhs) const;
    /// zero all elements
    VarMat<T>& zero() { vv.zero(); return *this; }

    /// VarMat multiplication
    const VarMat<T> operator*(const VarMat<T>& B) const;
    /// left-multiply a vector
    template<typename U, typename V>
    const VarVec<V> lMultiply(const VarVec<U>& v) const;
    /// right-multiply a vector
    template<typename U, typename V>
    const VarVec<V> rMultiply(const VarVec<U>& v) const;
    /// vector multiplication, when all objects are of same type
    const VarVec<T> operator*(const VarVec<T>& v) const { return lMultiply<T,T>(v); }

    /// Dump binary data to file
    void writeToFile(ostream& o) const;
    /// Read binary data from file
    static VarMat<T> readFromFile(std::istream& s);

private:

    size_t M;
    size_t N;
    VarVec<T> vv;

    /// step in inversion process
    void subinvert(size_t n);
};

template<typename T>
VarMat<T> VarMat<T>::random(size_t m, size_t n) {
    VarMat<T> foo(m,n);
    for(size_t i=0; i<foo.size(); i++)
        foo[i] = 0.1+T(rand())/T(RAND_MAX);
    return foo;
}

template<typename T>
VarMat<T> VarMat<T>::identity(size_t n, const T& one, const T& zero) {
    VarMat<T> foo(n,n,zero);
    for(size_t i=0; i<n; i++)
        foo(i,i) = one;
    return foo;
}

template<typename T>
VarMat<T> VarMat<T>::transposed() const {
    VarMat<T> foo = VarMat(N,M);
    for(size_t r=0; r<M; r++)
        for(size_t c=0; c<N; c++)
            foo(c,r) = (*this)(r,c);
    return foo;
}

template<typename T>
const VarMat<T> VarMat<T>::operator-() const {
    VarMat<T> foo = VarMat(M,N);
    for(size_t i=0; i<M*N; i++)
        foo[i] = -(*this)[i];
    return foo;
}

template<typename T>
const VarMat<T> VarMat<T>::operator*(const T& c) const {
    VarMat<T> foo = *this;
    foo *= c;
    return foo;
}

template<typename T>
const VarMat<T> VarMat<T>::operator/(const T& c) const {
    VarMat<T> foo = *this;
    foo /= c;
    return foo;
}

template<typename T>
const VarMat<T> VarMat<T>::operator+(const VarMat<T>& rhs) const {
    VarMat<T> foo = *this;
    foo += rhs;
    return foo;
}


template<typename T>
const VarMat<T> VarMat<T>::operator-(const VarMat<T>& rhs) const {
    VarMat<T> foo = *this;
    foo -= rhs;
    return foo;
}

template<typename T>
const VarMat<T> VarMat<T>::operator*(const VarMat<T>& B) const {
    if(B.nRows() != N)
        throw(DimensionMismatchError());
    size_t L = B.nCols();
    VarMat<T> C = VarMat<T>(M,L);
    for(size_t r=0; r<M; r++) {
        for(size_t c=0; c<L; c++) {
            C(r,c) = (*this)(r,0)*B(0,c);
            for(size_t i=1; i<N; i++)
                C(r,c) += (*this)(r,i)*B(i,c);
        }
    }
    return C;
}

template<typename T>
template<typename U, typename V>
const VarVec<V> VarMat<T>::lMultiply(const VarVec<U>& v) const {
    if(v.size() != N)
        throw(DimensionMismatchError());
    VarVec<V> a;
    for(size_t r=0; r<M; r++) {
        a.push_back((*this)(r,0)*v[0]);
        for(size_t c=1; c<N; c++)
            a.back() += (*this)(r,c)*v[c];
    }
    return a;
}

template<typename T>
template<typename U, typename V>
const VarVec<V> VarMat<T>::rMultiply(const VarVec<U>& v) const {
    if(v.size() != M)
        throw(DimensionMismatchError());
    VarVec<V> a;
    if(!size()) return a;
    for(size_t c=0; c<N; c++) {
        a.push_back(v[0] * (*this)(0,c));
        for(size_t r=1; r<M; r++)
            a.back() += v[r] * (*this)(r,c);
    }
    return a;
}

template<typename T>
T VarMat<T>::trace() const {
    if(!size()) return T();
    T s = vv[0];
    for(size_t i=1; i<std::min(M,N); i++)
        s += (*this)(i,i);
    return  s;
}

template<typename T>
const VarMat<T>& VarMat<T>::resize(size_t m, size_t n) {
    // change column dimension
    N = n;
    vv.getData().resize(M*N);

    // change row dimension
    if(m != M) {
        VarVec<T> vnew;
        for(n=0; n<N; n++)
            for(size_t i=0; i<m; i++)
                vnew.push_back( i<M? (*this)(i,n) : T() );
        vv = vnew;
        M = m;
    }

    return *this;
}

template<typename T>
VarVec<T> VarMat<T>::getRow(size_t r) const {
    VarVec<T> v(nCols());
    for(size_t c=0; c<nCols(); c++) v[c] = (*this)(r,c);
    return v;
}

template<typename T>
VarVec<T> VarMat<T>::getCol(size_t c) const {
    VarVec<T> v(nRows());
    for(size_t r=0; r<nRows(); r++) v[r] = (*this)(r,c);
    return v;
}

template<typename T>
VarVec<T> VarMat<T>::getRowSum() const {
    VarVec<T> v(nRows());
    for(size_t r=0; r<nRows(); r++)
        for(size_t c=0; c<nCols(); c++)
            v[r] += (*this)(r,c);
    return v;
}

template<typename T>
VarVec<T> VarMat<T>::getColSum() const {
    VarVec<T> v(nCols());
    for(size_t r=0; r<nRows(); r++)
        for(size_t c=0; c<nCols(); c++)
            v[c] += (*this)(r,c);
    return v;
}

template<typename T>
const VarMat<T>& VarMat<T>::invert() {
    if(M != N)
        throw(DimensionMismatchError());
    subinvert(0);
    return *this;
}

namespace VarMat_element_inversion {

    template<typename T>
    inline void invert_element(T& t) { t.invert(); }
    template<>
    inline void invert_element(float& t) { t = 1.0/t; }
    template<>
    inline void invert_element(double& t) { t = 1.0/t; }

}

template<typename T>
void VarMat<T>::subinvert(size_t n) {

    // invert the first cell
    T& firstcell = (*this)(n,n);
    VarMat_element_inversion::invert_element(firstcell);
    for(size_t i=n+1; i<M; i++)
        (*this)(n,i) *= firstcell;

    // use to clear first column
    for(size_t r=n+1; r<M; r++) {
        T& m0 = (*this)(r,n);
        for(size_t c=n+1; c<M; c++)
            (*this)(r,c) -= (*this)(n,c)*m0;
        m0 *= -firstcell;
    }

    if(n==M-1)
        return;

    //invert the subVarMat
    subinvert(n+1);

    // temporary space allocation
    vector<T> subvec = vector<T>(M-n-1);

    // first column gets multiplied by inverting subVarMat
    for(size_t r=n+1; r<M; r++)
        subvec[r-n-1] = (*this)(r,n);
    for(size_t r=n+1; r<M; r++) {
        (*this)(r,n) = (*this)(r,n+1)*subvec[0];
        for(size_t c=n+2; c<M; c++)
            (*this)(r,n) += (*this)(r,c)*subvec[c-n-1];
    }

    //finish off by cleaning first row
    for(size_t c=n+1; c<M; c++)
        subvec[c-n-1] = (*this)(n,c);
    for(size_t c=n; c<M; c++) {
        if(c>n)
            (*this)(n,c) = -(*this)(n+1,c) * subvec[0];
        else
            (*this)(n,c) -= (*this)(n+1,c) * subvec[0];
        for(size_t r=n+2; r<M; r++)
            (*this)(n,c) -= (*this)(r,c) * subvec[r-n-1];
    }

}

/// string output representation for VarMat; TODO sensible output for complex types
template<typename T>
ostream& operator<<(ostream& o, const VarMat<T>& A) {
    for(size_t r=0; r<A.nRows(); r++) {
        o << "[ ";
        for(size_t c=0; c<A.nCols(); c++) {
            o << A(r,c);
            if(c+1<A.nCols()) o << ", ";
        }
        o << " ],\n";
    }
    return o;
}

template<typename T>
void VarMat<T>::writeToFile(ostream& o) const {
    writeString("(VarMat_"+std::to_string(sizeof(T))+")",o);
    o.write((char*)&M, sizeof(M));
    o.write((char*)&N, sizeof(N));
    vv.writeToFile(o);
    writeString("(/VarMat_"+std::to_string(sizeof(T))+")",o);
}

template<typename T>
VarMat<T> VarMat<T>::readFromFile(std::istream& s) {
    checkString("(VarMat_"+std::to_string(sizeof(T))+")",s);
    VarMat<T> foo;
    s.read((char*)&foo.M, sizeof(foo.M));
    s.read((char*)&foo.N, sizeof(foo.N));
    foo.vv = VarVec<T>::readFromFile(s);
    assert(foo.M*foo.N == foo.vv.size());
    checkString("(/VarMat_"+std::to_string(sizeof(T))+")",s);
    return foo;
}

template<typename T, typename U>
VarMat<U> convertType(const VarMat<T>& v) {
    VarMat<U> u(v.nRows(),v.nCols());
    for(size_t i=0; i<v.size(); i++) u[i] = v[i];
    return u;
}

/// outer product of vectors
template<typename T>
VarMat<T> outer(const VarVec<T>& a, const VarVec<T>& b) {
    VarMat<T> M(a.size(),b.size());
    for(size_t i=0; i<a.size(); i++)
        for(size_t j=0; j<b.size(); j++)
            M(i,j) = a[i]*b[j];
    return M;
}

#endif
