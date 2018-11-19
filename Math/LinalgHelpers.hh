/// \file LinalgHelpers.hh Utility routines using GSL linear algebra
// Michael P. Mendenhall, 2018

#ifndef LINALGHELPERS_HH
#define LINALGHELPERS_HH

#include <gsl/gsl_linalg.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_eigen.h>
#include <utility>
#include <cassert>

/// right-multiply (scale columns) of M by diagonal matrix D
void rmul_diag(gsl_matrix* M, const gsl_vector* D);

/// right-divide (scale columns) of M by diagonal matrix D
void rdiv_diag(gsl_matrix* M, const gsl_vector* D);

/// left-multiply (scale rows) of M by diagonal matrix D
void lmul_diag(gsl_matrix* M, const gsl_vector* D);

/// Allocated workspace for N*N matrix operations
class MnnWorkspace {
public:
    /// Constructor
    MnnWorkspace(size_t n): N(n), M(gsl_matrix_alloc(N,N)) { }
    /// Destructor
    ~MnnWorkspace() { gsl_matrix_free(M); }

    /// Rank-2 symmetric A -> A A^T (default fill lower triangle; also option to transpose for A^T A)
    void square(gsl_matrix*& A, CBLAS_UPLO_t ul = CblasLower, CBLAS_TRANSPOSE_t t = CblasNoTrans) {
        assert(A && A->size1 == N && A->size2 == N);
        gsl_blas_dsyrk(ul, t, 1., A, 0., M);
        std::swap(A,M);
    }

    const size_t N; ///< matrix dimension

protected:
    gsl_matrix* M;  ///< NxN workspace
};

/// Helper workspace for SVD A(M,N) = U(M,N) S(N,N) V^T(N,N)
class SVDWorkspace {
public:
    /// Constructor
    SVDWorkspace(size_t n, size_t m);
    /// Destructor
    ~SVDWorkspace();

    /// Perform decomposition; outputs U in A
    int SVD(gsl_matrix* A) { return gsl_linalg_SV_decomp(A, V, S, w); }

    const size_t N;     ///< input columns
    const size_t M;     ///< input rows
    gsl_matrix* V;      ///< NxN orthogonal matrix
    gsl_vector* S;      ///< diagonal of NxN singular-values matrix

protected:
    gsl_vector* w;      ///< workspace N
};

/// Allocated workspace for NxN Eigenvector decompositions
class EigNWorkspace {
public:
    /// Constructor
    EigNWorkspace(size_t n): evec(gsl_matrix_alloc(n,n)), W(gsl_eigen_symmv_alloc(n)) { }
    /// Destructor
    ~EigNWorkspace() { gsl_matrix_free(evec); gsl_eigen_symmv_free(W); }

    /// Decompose symmetric (lower-triangle) A -> U D U^T; return eigenvectors in A -> U columns
    void decompSymm(gsl_matrix*& A, gsl_vector* D) { gsl_eigen_symmv(A, D, evec, W); std::swap(evec,A); }

protected:
    gsl_matrix* evec;               ///< eigenvector storage
    gsl_eigen_symmv_workspace* W;   ///< symmetrix matrix eigendecomposition workspace
};

/// Zero out triangle above/below diagonal (after triangular matrix ops leaving junk here)
void zeroTriangle(CBLAS_UPLO_t uplo, gsl_matrix* A);

/// Re-usable workspace for projecting an N-dimensional ellipsoid into an M-dimensional affine subspace
// formula from Stephen B. Pope, "Algorithms for Ellipsoids," Cornell University Report FDA-08-01 (2008)
class ellipse_affine_projector: public SVDWorkspace {
public:
    /// Constructor
    ellipse_affine_projector(size_t n, size_t m);
    /// Destructor
    ~ellipse_affine_projector();

    /// Set T to vectors along specified axes
    template<typename V>
    void setAxes(const V& a) {
        gsl_matrix_set_zero(TT);
        for(size_t i=0; i<M; i++) gsl_matrix_set(TT, i, a[i], 1.);
    }

    /// Project from Cholesky form L(lower) to PCA U S^-1; outputs P = U/sigma and S = 1/sigma
    void projectL(const gsl_matrix* L);

    gsl_matrix* TT;     ///< T^T [M,N] orthogonal matrix defining affine subspace in rows
    gsl_matrix* P;      ///< P[M,M] result projection principal axes in columns

protected:
    gsl_matrix* Mmn;    ///< MxN workspace
    gsl_matrix* Mnn;    ///< NxN workspace
};

/// print vector to stdout
void display(const gsl_vector* v);
/// print matrix to stdout
void display(const gsl_matrix* M);

#endif
