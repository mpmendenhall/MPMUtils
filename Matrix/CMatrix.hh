/* 
 * CMatrix.hh, part of the RotationShield program
 * Copyright (c) 2007-2014 Michael P. Mendenhall
 *
 * This code uses the FFTW3 library for Fourier transforms, http://www.fftw.org/
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

/// \file "CMatrix.hh" \brief Circulant matrices
#ifndef CMATRIX_HH
/// Make sure this header is only loaded once
#define CMATRIX_HH

#include <iostream>
#include <fftw3.h>
#include <map>
#include <complex.h>
#include "VarVec.hh"
#include "BinaryOutputObject.hh"

using std::vector;
using std::map;
using std::complex;

/// Stores fftw data for FFT'ing
class cmatrix_fft {
public:
    /// constructor
    cmatrix_fft(unsigned int m);
    /// destructor
    ~cmatrix_fft() { delete[] realspace; delete[] kspace; }
    
    const unsigned int M;       ///< number of elements
    fftw_plan forwardplan;      ///< FFTW data for forward Fourier Transforms of this size
    fftw_plan reverseplan;      ///< FFTW data for inverse Fourier Transforms of this size
    double* realspace;          ///< array for holding real-space side of transform data
    complex<double>* kspace;    ///< array for holding kspace-side of transform data
    
    /// get FFTer for dimension m
    static cmatrix_fft& get_ffter(unsigned int m);
    
protected:
    
    static map<unsigned int,cmatrix_fft*> ffters;  ///< loaded FFTers
};

namespace VarVec_element_IO {
    template<>
    inline void writeToFile(const complex<double>& t, ostream& o) { o.write((char*)&t, sizeof(t)); }
    
    template<>
    inline complex<double> readFromFile(std::istream& s) { complex<double> x; s.read((char*)&x, sizeof(x)); return x; }
}

/// Circulant matrices
/** A circulant matrix is a square matrix in which each row is a cyclic permutation by one of the previous row, e.g.
 * \f$ \left| \begin{array}{ccc} a & b & c \\ c & a & b \\ b & c & a \end{array} \right| \f$.
 * These matrices are convolution operators on vectors, thus they commute and are diagonalized by a Fourier transform.
 * The CMatrix class transparently handles converting circulant matrices into and out of the
 * Fourier basis, allowing for computationally efficient handling of matrix operations
 * (multiplication, inversion, etc.) of circulant matrices.
 * Note, the internal data representation is the transpose of the matrix as defined above;
 * the necessary permutation of component order is automatically applied for vector multiplication.
 * The FFTs are performed by the <a href="http://www.fftw.org">FFTW library</a>,
 * which pre-calculates plans to expedite FFT'ing specific length data arrays. The CMatrix class keeps a cache of
 * the FFTW data needed for each size of CMatrix instantiated (which could become inefficient if a wide variety of
 * CMatrix sizes are used in the same code, but is suitable for this application where only one size of CMatrix is used
 * for a particular interaction symmetry).*/
class CMatrix: public BinaryOutputObject {
public:
    /// Constructor
    CMatrix(unsigned int m = 0): M(m), data(M,0.), kdata(M/2+1,0.), has_realspace(true), has_kspace(true) { }
    
    /*
     * /// Save matrix to a file (to be read by readFromFile())
     * void writeToFile(ostream& o) const;
     * /// Read matrix from a file written by writeToFile()
     * static CMatrix readFromFile(std::istream& s);
     */
    
    /// generate an identity CMatrix
    static CMatrix identity(unsigned int m);
    /// Fill this CMatrix with random numbers in [0,1]
    static CMatrix random(unsigned int m);
    
    unsigned int nRows() const { return M; }
    unsigned int nCols() const { return M; }
    unsigned int size() const { return M*M; }
    
    /// Print this CMatrix to stdout
    void display() const;
    /// Print kspace data for this CMatrix to stdout
    void displayK() const;
    
    /// immutable element access
    double operator[](unsigned int n) const;
    /// mutable element access
    double& operator[](unsigned int n);
    
    /// L2 (Spectral) norm of circulant matrix
    double norm_L2() const;
    /// determinant of circulant matrix
    double det() const;
    /// trace of circulant matrix
    double trace() const;
    
    /// Return a pointer to the CMatrix's Fourier representation
    vector< complex<double> >& getKData();
    /// Return a pointer to the CMatrix's Fourier representation (read only)
    const vector< complex<double> >&  getKData() const;
    /// Return a pointer to the CMatrix's real-space representation
    vector<double>&  getRealData();
    /// Return a pointer to the CMatrix's real-space representation (read only)
    const vector<double>& getRealData() const;
    
    /// Calculate the inverse of this CMatrix
    const CMatrix inverse() const;
    /// Invert this CMatrix inplace
    CMatrix& invert();
    /// Return the transpose of this CMatrix
    const CMatrix transpose() const;
    
    /// unary minus
    const CMatrix operator-() const;
    
    /// Product with a scalar, inplace
    CMatrix& operator*=(double c);
    /// Product with another CMatrix (circulant matrices are commutative!)
    CMatrix& operator*=(const CMatrix& m);
    /// Product with a scalar
    const CMatrix operator*(double c) const;
    /// Product with another CMatrix (circulant matrices are commutative!)
    const CMatrix operator*(const CMatrix& m) const;
    /// Multiply a vector on the right
    const VarVec<double> operator*(const VarVec<double>& v) const;
    
    /// add another CMatrix to this one
    CMatrix& operator+=(const CMatrix& rhs);
    /// sum of two CMatrices
    const CMatrix operator+(const CMatrix& rhs) const;
    /// subtract another CMatrix from this one
    CMatrix& operator-=(const CMatrix& rhs);
    /// difference of two CMatrices
    const CMatrix operator-(const CMatrix& rhs) const;
    
    /// Print the rth row of the matrix to stdout
    void printRow(int r) const;
    
    /// Dump binary data to file
    void writeToFile(ostream& o) const;
    /// Read binary data from file
    static CMatrix readFromFile(std::istream& s);
    
private:
    
    unsigned int M;     ///< number of cycles
    
    /// calculate K-space data from real space
    void calculateKData() const;
    /// calculate real-space data from K-space
    void calculateRealData() const;
    
    /// zero all entries in this CMatrix
    void zero() const;
    
    mutable vector<double> data;                   ///< real-space data
    mutable vector< complex<double> > kdata;       ///< K-space data
    mutable bool has_realspace;                         ///< whether the real-space representation of this matrix has been calculated
    mutable bool has_kspace;                            ///< whether the k-space representation of this matrix has been calculated
};

ostream& operator<<(ostream& o, const CMatrix& m);

#endif
