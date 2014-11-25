/* 
 * LAPACKE_Matrix.hh, part of the MPMUtils package.
 * Copyright (c) 2007-2014 Michael P. Mendenhall
 *
 * This code uses the LAPACKE C interface to LAPACK;
 * see http://www.netlib.org/lapack/lapacke.html
 * and the GSL interface to CBLAS, https://www.gnu.org/software/gsl/
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

/// \file MAPACKE_Matrix.hh \brief Convenience interface to LAPACK matrix operations
#ifndef LAPACKE_MATRIX_HH
/// Make sure this header is only loaded once
#define LAPACKE_MATRIX_HH

#ifdef WITH_LAPACKE

#include "VarMat.hh"
#include <complex.h>
#define lapack_complex_float  std::complex<float>
#define lapack_complex_double  std::complex<double>
#include <lapacke.h>
#include "BinaryOutputObject.hh"
#include <algorithm>
#include <cassert>
#include <gsl/gsl_cblas.h>

using std::cout;

/// virtual base class for encapsulating BLAS operations on various matrix types
template<typename T>
class Mat_Ops {
public:
    /// constructor
    Mat_Ops() {}
    /// destructor
    virtual ~Mat_Ops() {}
    
    /// multiplication
    virtual VarMat<T>* mul(const VarMat<T>& A,
                           const VarMat<T>& B,
                           CBLAS_TRANSPOSE opA = CblasNoTrans,
                           CBLAS_TRANSPOSE opB = CblasNoTrans,
                           VarMat<T>* C = NULL,
                           T alpha = 1, T beta = 0) const = 0;
};

/// real-valued matrix operations
template<typename T>
class Mat_Ops_Real: public Mat_Ops<T> {
public:
    
    /// form a new matrix by multiplication C = alpha * o(A)*o(B) + beta * C
    virtual VarMat<T>* mul(const VarMat<T>& A,
                           const VarMat<T>& B,
                           CBLAS_TRANSPOSE opA = CblasNoTrans,
                           CBLAS_TRANSPOSE opB = CblasNoTrans,
                           VarMat<T>* C = NULL,
                           T alpha = 1, T beta = 0) const {
                               
                               const unsigned int opA_rows = A.nDim(opA == CblasNoTrans);
                               const unsigned int opA_cols = A.nDim(opA != CblasNoTrans);
                               const unsigned int opB_rows = B.nDim(opB == CblasNoTrans);
                               const unsigned int opB_cols = B.nDim(opB != CblasNoTrans);
                               
                               assert(opA_cols == opB_rows);
                               if(!C) C = new VarMat<T>(opA_rows, opB_cols);
                               assert(C->nRows() == opA_rows && C->nCols() == opB_cols);
                               
                               (*f_gemm)(CblasColMajor, // data order, CblasColMajor or CblasRowMajor
                                         opA,           // op(A) = CblasNoTrans, CblasTrans, CblasConjTrans
                                         opB,           // op(B) = CblasNoTrans, CblasTrans, CblasConjTrans
                                         C->nRows(),    // rows of op(A) and C
                                         C->nCols(),    // columns of op(B) and C
                                         opA_cols,      // columns of op(A), rows of op(B)
                                         alpha,         // scalar alpha
                                         &A[0],         // matrix A's data
                                         A.nRows(),     // leading dimension of A
                                         &B[0],         // matrix B's data
                                         B.nRows(),     // B's leading dimension
                                         beta,          // scalar beta
                                         &(*C)[0],      // output matrix C data
                                         C->nRows()     // leading dimension of C
                               );
                               return C;
                           }
                           
    /// matrix multiply BLAS function
    static void (*f_gemm)(const enum CBLAS_ORDER, const enum CBLAS_TRANSPOSE, const enum CBLAS_TRANSPOSE,
                          const int, const int, const int, const T, const T*, const int lda,
                          const T*, const int, const T, T*, const int);
};

/// complex-valued matrix operations
template<typename CT>
class Mat_Ops_Complex: public Mat_Ops<CT> {
public:
    
    /// form a new matrix by multiplication C = alpha * o(A)*o(B) + beta * C
    virtual VarMat<CT>* mul(const VarMat<CT>& A,
                            const VarMat<CT>& B,
                            CBLAS_TRANSPOSE opA = CblasNoTrans,
                            CBLAS_TRANSPOSE opB = CblasNoTrans,
                            VarMat<CT>* C = NULL,
                            CT alpha = 1, CT beta = 0) const {
                                
                                const unsigned int opA_rows = A.nDim(opA == CblasNoTrans);
                                const unsigned int opA_cols = A.nDim(opA != CblasNoTrans);
                                const unsigned int opB_rows = B.nDim(opB == CblasNoTrans);
                                const unsigned int opB_cols = B.nDim(opB != CblasNoTrans);
                                
                                assert(opA_cols == opB_rows);
                                if(!C) C = new VarMat<CT>(opA_rows, opB_cols);
                                assert(C->nRows() == opA_rows && C->nCols() == opB_cols);
                                
                                (*f_gemm)(CblasColMajor,        // data order, CblasColMajor or CblasRowMajor
                                          opA,                  // op(A) = CblasNoTrans, CblasTrans, CblasConjTrans
                                          opB,                  // op(B) = CblasNoTrans, CblasTrans, CblasConjTrans
                                          C->nRows(),           // rows of op(A) and C
                                          C->nCols(),           // columns of op(B) and C
                                          opA_cols,             // columns of op(A), rows of op(B)
                                          &alpha,               // scalar alpha
                                          &A[0],                // matrix A's data
                                          A.nRows(),            // leading dimension of A
                                          &B[0],                // matrix B's data
                                          B.nRows(),            // B's leading dimension
                                          &beta,                // scalar beta
                                          &(*C)[0],             // output matrix C data
                                          C->nRows()            // leading dimension of C
                                );
                                return C;
                            }
                            
                            /// matrix multiply BLAS function
                            static void (*f_gemm)(const enum CBLAS_ORDER, const enum CBLAS_TRANSPOSE, const enum CBLAS_TRANSPOSE,
                                                  const int, const int, const int, const void*, const void*, const int lda,
                                                  const void*, const int, const void*, void*, const int);
};



/// Templatized wrapper for SVD of matrix A = U S V^T
template<typename T, typename CT>
class LAPACKE_Matrix_SVD: public BinaryOutputObject {
public:
    /// Constructor
    LAPACKE_Matrix_SVD(VarMat<CT>& A): S(std::min(A.nRows(), A.nCols()),1), U(A.nRows(), S.nRows()), VT(S.nRows(), A.nCols()), PsI(NULL), PsI_epsilon(0) {
        
        lapack_int info;
        bool verbose = false;
        
        char diag = (A.nRows() >= A.nCols() ? 'U':'L');         // upper or lower diagonal reduction, depending on A's dimensions
        VarMat<T> e(std::max(S.nRows()-1,(unsigned int)1),1);   // holder for secondary diagonal
        CT* tauQ = new CT[S.nRows()];
        CT* tauP = new CT[S.nRows()];
        
        // overall reduce general A = (U*Q) * S * (P^H * V^T)
        
        // decomposes m*n A = Q B P^H,  Q and P unitary, B bidiagonal of min(m,n)*min(m,n)
        // over-writes A with B on diagonals, P above, and Q below.
        info = (*f_gebrd)(LAPACK_COL_MAJOR,     // LAPACK_COL_MAJOR or LAPACK_ROW_MAJOR data ordering,
                          A.nRows(),            // m: number of rows in A
                          A.nCols(),            // n: number of columns in A
                          &A[0],                // matrix A
                          A.nRows(),            // lda: leading dimension of A, >= max(1,m)
                          &S[0],                // diagonal elements of B, dimension >= max(1, min(m, n))
                          &e[0],                // off-diagonal elements of B, dimension >= max(1, min(m, n) - 1)
                          tauQ,                 // array with extra into on Q, dimension >= max(1, min(m, n))
                          tauP                  // array with extra info on P, dimension >= max(1, min(m, n))
        );
        assert(!info);
        
        if(verbose) {
            cout << "Bi-diagonalizing A:\n\n";
            cout << "Main diagonal:\n" << S << "\n";
            cout << "Secondary diagonal:\n" << e << "\n";
        }
        
        // extract bi-diagonalizing matrices, truncated to useful dimension
        U = A;
        VT = A;
        
        info = (*f_orgbr)(LAPACK_COL_MAJOR,     // LAPACK_COL_MAJOR or LAPACK_ROW_MAJOR data ordering,
                          'Q',                  // extract 'Q' or 'P' (P^T)
                          U.nRows(),            // rows in extracted matrix
                          S.nRows(),            // columns in extracted matrix
                          A.nCols(),            // columns for Q, rows for P of original matrix reduced by ?gebrd
                          &U[0],                // (copy of) return matrix A of ?gebrd, will be over-written
                          U.nRows(),            // leading dimension of "A"
                          tauQ                  // extra return array tauQ or tauP from ?gebrd
        );
        assert(!info);
        
        info = (*f_orgbr)(LAPACK_COL_MAJOR,     // LAPACK_COL_MAJOR or LAPACK_ROW_MAJOR data ordering,
                          'P',                  // extract 'Q' or 'P' (P^T)
                          S.nRows(),            // rows in extracted matrix
                          VT.nCols(),           // columns in extracted matrix
                          A.nRows(),            // columns for Q, rows for P of original matrix reduced by ?gebrd
                          &VT[0],               // (copy of) return matrix A of ?gebrd, will be over-written
                          VT.nRows(),           // leading dimension of "A"
                          tauP                  // extra return array tauQ or tauP from ?gebrd
        );
        assert(!info);
        
        // trim U, VT to useful content
        U.resize(A.nRows(),S.nRows());
        VT.resize(S.nRows(),A.nCols());
        
        if(verbose) {
            cout << "Q left bidiagonalizer:\n" << U << "\n";
            cout << "P right bidiagonalizer:\n" << VT << "\n";
            cout << "\nSingular Value Decomposition:\n\n";
        }
        
        // no longer need these
        delete[] tauQ;
        delete[] tauP;
        
        // decompose B = U2 S V2^H, Q and P orthogonal singular vectors, S diagonal singular values
        // overwrites d = S; destroys e
        info = (*f_bdsqr)(LAPACK_COL_MAJOR,     // LAPACK_COL_MAJOR or LAPACK_ROW_MAJOR data ordering
                          diag,                 // 'U'pper or 'L'ower bidiagonal
                          S.nRows(),            // order >= 0 of matrix B
                          A.nCols(),            // ncvt: number of columns of V^T, right singular vectors to calculate
                          A.nRows(),            // nru: number of rows of U, left singular vectors to calculate
                          0,                    // ncc: number of columns in C used for QH*C; set 0 if no "C" supplied
                          &S[0],                // d: diagonal elements of B; must be size >= max(1,n)
                          &e[0],                // e: n-1 off-diagonal elements of B; must be size >= max(1,n)
                          &VT[0],               // n * ncvt matrix for right singular vectors; unused if ncvt=0
                          VT.nRows(),           // leading dimension of VT; >= max(1,n) for ncvt>0; >= 1 otherwise.
                          &U[0],                // nru * n unit matrix U; unused if nru=0
                          U.nRows(),            // leading dimension of U; >= max(1,nru)
                          NULL,                 // matrix for calculating Q^H*C; second dimension >= max(1,ncc); unused if ncc = 0
                          1                     // leading dimension of C; >= max(1,n) if ncc>0; >=1 otherwise
        );
        assert(!info);
        
        if(verbose) {
            cout << "Left singular vectors:\n" << U << "\n";
            cout << "Singular values:\n" << S << "\n";
            cout << "Right singular vectors:\n" << VT << "\n";
        }
        
    }
    
    /// destructor
    ~LAPACKE_Matrix_SVD() { if(PsI) delete PsI; }
    
    /// Calculate pseudo-inverse, discarding singular values <= epsilon
    const VarMat<CT>& calc_pseudo_inverse(T epsilon = 0) {
        if(PsI && PsI_epsilon==epsilon) return *PsI;
        if(PsI) delete PsI;
        PsI_epsilon = epsilon;
        
        // U * (S^-1)
        VarMat<CT> USI = U;
        for(unsigned int n=0; n<USI.nCols(); n++) {
            T psi = (epsilon >= 0 ? (fabs(S[n]) <= epsilon ? 0 : 1./S[n]) :
            (fabs(S[n]) <= fabs(epsilon) ? 1 : 0));
            for(unsigned int m=0; m<USI.nRows(); m++) {
                USI(m,n) *= psi;
            }
        }
        PsI = myOps->mul(VT, USI, CblasConjTrans, CblasConjTrans);
        return *PsI;
    }
    
    /// get list of singular values
    const VarMat<T>& singular_values() const { return S; }
    
    /// get enumerated right singular vector
    VarVec<CT> getRightSVec(unsigned int i) { return VT.getRow(i); }
    /// get enumerated left singular vector
    VarVec<CT> getLeftSVec(unsigned int i) { return U.getCol(i); }
    
    /// Dump binary data to file
    void writeToFile(ostream& o) const {
        writeString("(LAPACKE_Matrix_SVD_"+std::to_string(sizeof(CT))+")",o);
        S.writeToFile(o);
        U.writeToFile(o);
        VT.writeToFile(o);
        o.write((char*)&PsI,            sizeof(PsI));
        o.write((char*)&PsI_epsilon,    sizeof(PsI_epsilon));
        if(PsI) PsI->writeToFile(o);
        writeString("(/LAPACKE_Matrix_SVD_"+std::to_string(sizeof(CT))+")",o);
    }
    
    /// Read binary data from file
    static LAPACKE_Matrix_SVD* readFromFile(std::istream& s) {
        LAPACKE_Matrix_SVD* foo = new LAPACKE_Matrix_SVD();
        
        checkString("(LAPACKE_Matrix_SVD_"+std::to_string(sizeof(CT))+")",s);
        foo->S = VarMat<T>::readFromFile(s);
        foo->U = VarMat<CT>::readFromFile(s);
        foo->VT = VarMat<CT>::readFromFile(s);
        s.read((char*)&foo->PsI,        sizeof(foo->PsI));
        s.read((char*)&foo->PsI_epsilon,sizeof(foo->PsI_epsilon));
        if(foo->PsI) {
            foo->PsI = new VarMat<CT>();
            *foo->PsI = VarMat<CT>::readFromFile(s);
        }
        checkString("(/LAPACKE_Matrix_SVD_"+std::to_string(sizeof(CT))+")",s);
        return foo;
    }
    
protected:
    
    /// non-calculating constructor
    LAPACKE_Matrix_SVD(): PsI(NULL), PsI_epsilon(0) {}
    
    VarMat<T> S;        ///< singular values diagonal
    VarMat<CT> U;       ///< left singular vectors
    VarMat<CT> VT;      ///< right singular vectors
    VarMat<CT>* PsI;    ///< pseudo-inverse
    T PsI_epsilon;      ///< threshold for singular vectors
    
    /// appropriate data type version of xGEBRD bidiagonal reduction
    static lapack_int (*f_gebrd)(int, lapack_int, lapack_int, CT*, lapack_int, T*, T*, CT*, CT*);
    /// appropriate data type version of orgbr/ungbr, unpacks bidiagonal decomposition matrices
    static lapack_int (*f_orgbr)(int, char, lapack_int, lapack_int, lapack_int, CT*, lapack_int, const CT*);
    /// appropriate data type version of zBDSQR SVD from bidiagonal
    static lapack_int (*f_bdsqr)(int, char, lapack_int, lapack_int, lapack_int, lapack_int, T*, T*, CT*, lapack_int, CT*, lapack_int, CT*, lapack_int);
    
    /// bundle of other appropriate matrix operations
    static Mat_Ops<CT>* myOps;
};

#endif
#endif

