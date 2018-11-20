/// \file LinalgHelpers.hh Utility routines using GSL linear algebra
// Michael P. Mendenhall, 2018

#ifndef LINALGHELPERS_HH
#define LINALGHELPERS_HH

#include <gsl/gsl_linalg.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_eigen.h>
#include <gsl/gsl_blas.h>
#include <utility>
#include <cassert>
#include <vector>
using std::vector;

/// print vector to stdout
void displayV(const gsl_vector* v);
/// print matrix to stdout
void displayM(const gsl_matrix* M);

/// right-multiply (scale columns) of M by diagonal matrix D
void rmul_diag(gsl_matrix* M, const gsl_vector* D);
/// right-divide (scale columns) of M by diagonal matrix D
void rdiv_diag(gsl_matrix* M, const gsl_vector* D);
/// left-multiply (scale rows) of M by diagonal matrix D
void lmul_diag(gsl_matrix* M, const gsl_vector* D);

/// Zero out triangle above/below diagonal (after triangular matrix ops leaving junk here)
void zeroTriangle(CBLAS_UPLO_t uplo, gsl_matrix* A);
/// Fill in specified half of symmetric matrix from other side
void fillSymmetric(CBLAS_UPLO_t uplo, gsl_matrix* A);

/// sum x x^T points into symmetric matrix (lower triangle)
template<typename V>
void sumSymm(gsl_matrix* A, gsl_vector* vn, const V& vp) {
    for(auto& p: vp) {
        for(size_t i=0; i<vn->size; i++) gsl_vector_set(vn, i, p[i]);
        gsl_blas_dsyr(CblasLower, 1., vn, A);
    }
}
/// resize (allocate if needed) gsl vector
void resize(gsl_vector*& g, size_t n);
/// fill gsl vector
template<typename YVec>
void vector2gsl(const YVec& v, gsl_vector*& g) {
    resize(g, v.size());
    for(size_t i=0; i<g->size; i++) gsl_vector_set(g,i,v[i]);
}
/// extract gsl vector
template<typename Vec>
static void gsl2vector(const gsl_vector* g, Vec& v) {
    if(!g) { v.clear(); return; }
    v.resize(g->size);
    for(size_t i=0; i<g->size; i++) v[i] = gsl_vector_get(g,i);
}




//----------------------------------------------------------
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



//----------------------------------------------------------
/// Workspace for symmetric NxN Eigenvector decomposition A -> U D U^T
class EigSymmWorkspace {
public:
    /// Constructor
    EigSymmWorkspace(size_t n): _N(n) { }
    /// Destructor
    ~EigSymmWorkspace() { if(evec) gsl_matrix_free(evec); if(W) gsl_eigen_symmv_free(W); }

    /// Decompose symmetric (lower-triangle) A -> U D U^T; return eigenvectors in A -> U columns
    void decompSymm(gsl_matrix*& A, gsl_vector* D) {
        if(!evec) evec = gsl_matrix_alloc(_N,_N);
        if(!W) W = gsl_eigen_symmv_alloc(_N);
        gsl_eigen_symmv(A, D, evec, W);
        std::swap(evec,A);
    }

    const size_t _N;

protected:
    gsl_matrix* evec = nullptr;             ///< eigenvector storage
    gsl_eigen_symmv_workspace* W = nullptr; ///< symmetrix matrix eigendecomposition workspace
};


//----------------------------------------------------------
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

#endif
