/// \file LinalgHelpers.hh Utility routines using GSL linear algebra
// -- Michael P. Mendenhall, 2018

#ifndef LINALGHELPERS_HH
#define LINALGHELPERS_HH

#include <gsl/gsl_linalg.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_eigen.h>
#include <gsl/gsl_blas.h>
#include <utility>
#include <cassert>
#include <vector>
#include <iostream>
using std::vector;

/// helper for memory-managing gsl_matrix
class gsl_matrix_wrapper {
public:
    /// Constructor with dimensions
    gsl_matrix_wrapper(size_t m = 0, size_t n = 0, bool c = true): M(m && n? c? gsl_matrix_calloc(m,n) : gsl_matrix_alloc(m,n) : nullptr) { }
    /// Copy constructor
    gsl_matrix_wrapper(const gsl_matrix_wrapper& w): M(nullptr) { *this = w; }
    /// Move constructor
    gsl_matrix_wrapper(gsl_matrix_wrapper&& other): M(nullptr) { *this = std::move(other); }
    /// Destructor
    ~gsl_matrix_wrapper() { if(M) gsl_matrix_free(M); }

    /// Copy assignment
    gsl_matrix_wrapper& operator=(const gsl_matrix_wrapper& w);
    /// Move assignemnt
    gsl_matrix_wrapper& operator=(gsl_matrix_wrapper&& other);

    /// easy element access
    double operator()(size_t i, size_t j) const { return gsl_matrix_get(M,i,j); }
    /// easy element access
    double& operator()(size_t i, size_t j) { return *gsl_matrix_ptr(M,i,j); }

    /// treat like gsl_matrix*
    operator const gsl_matrix*() const { return M; }
    /// treat like gsl_matrix*
    operator gsl_matrix*&() { return M; }
    /// treat like gsl_matrix*
    //gsl_matrix* operator->() { return M; }
    /// treat like gsl_matrix*
    const gsl_matrix* operator->() const { return M; }

protected:
    gsl_matrix* M;  ///< the matrix
};

/// serialization
std::ostream& operator<< (std::ostream &o, const gsl_matrix_wrapper& M);
/// deserialization
std::istream& operator>> (std::istream &i, gsl_matrix_wrapper& M);

/// helper for memory-managing gsl_vector
class gsl_vector_wrapper {
public:
    /// Constructor with dimensions
    gsl_vector_wrapper(size_t n = 0, bool c = true): v(n? c? gsl_vector_calloc(n) : gsl_vector_alloc(n) : nullptr) { }
    /// Copy constructor
    gsl_vector_wrapper(const gsl_vector_wrapper& w): v(nullptr) { *this = w; }
    /// Move constructor
    gsl_vector_wrapper(gsl_vector_wrapper&& other): v(nullptr) { *this = std::move(other); }
    /// Destructor
    ~gsl_vector_wrapper() { if(v) gsl_vector_free(v); }

    /// Copy assignment
    gsl_vector_wrapper& operator=(const gsl_vector_wrapper&);
    /// Move assignemnt
    gsl_vector_wrapper& operator=(gsl_vector_wrapper&& other);
    /// check pointer validity
    operator bool() const { return v; }

    /// easy element access
    double operator()(size_t i) const { return gsl_vector_get(v,i); }
    /// easy element access
    double& operator()(size_t i) { return *gsl_vector_ptr(v,i); }

    /// treat like gsl_vector*
    operator const gsl_vector*() const { return v; }
    /// treat like gsl_vector*
    operator gsl_vector*&() { return v; }
    /// treat like gsl_vector*
    //gsl_vector* operator->() { return v; }
    /// treat like gsl_vector*
    const gsl_vector* operator->() const { return v; }

protected:
    gsl_vector* v;      ///< the vector
};

/// serialization
std::ostream& operator<< (std::ostream &o, const gsl_vector_wrapper& v);
/// deserialization
std::istream& operator>> (std::istream &i, gsl_vector_wrapper& v);

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
/// scale columns by 1/||column||^2
void invert_colnorms(gsl_matrix* M);

/// Zero out triangle above/below diagonal (after triangular matrix ops leaving junk here)
void zeroTriangle(CBLAS_UPLO_t uplo, gsl_matrix* A);
/// Fill in specified half of symmetric matrix from other side
void fillSymmetric(CBLAS_UPLO_t uplo, gsl_matrix* A);

/// sum x x^T point into symmetric lower triangle
template<typename X>
void add_xTx(gsl_matrix* A, gsl_vector* vn, const X& x) {
    for(size_t i=0; i<vn->size; i++) gsl_vector_set(vn, i, x[i]);
    gsl_blas_dsyr(CblasLower, 1., vn, A);
}
/// sum x x^T points into symmetric matrix (lower triangle)
template<typename V>
void sumSymm(gsl_matrix* A, gsl_vector* vn, const V& vp) { for(auto& p: vp) add_xTx(A, vn, p); }

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
static void gsl2vector(const gsl_vector* gv, Vec& v) {
    if(!gv) { v.clear(); return; }
    v.resize(gv->size);
    for(size_t i=0; i<gv->size; i++) v[i] = gsl_vector_get(gv,i);
}




//----------------------------------------------------------
/// Helper workspace for SVD A(M,N) = U(M,N) S(N,N) V^T(N,N)
class SVDWorkspace {
public:
    /// Constructor
    explicit SVDWorkspace(size_t n): V(n,n), S(n), w(n) { }

    /// Perform decomposition; outputs U in A
    int SVD(gsl_matrix* A) { return gsl_linalg_SV_decomp(A, V, S, w); }

    gsl_matrix_wrapper V;   ///< NxN orthogonal matrix
    gsl_vector_wrapper S;   ///< diagonal of NxN singular-values matrix

protected:
    gsl_vector_wrapper w;   ///< workspace N
};



//----------------------------------------------------------
/// Workspace for symmetric NxN Eigenvector decomposition A -> U D U^T
class EigSymmWorkspace {
public:
    /// Constructor
    explicit EigSymmWorkspace(size_t n): _N(n) { }
    /// Copy constructor
    EigSymmWorkspace(const EigSymmWorkspace& E): _N(E._N) { }
    /// Destructor
    ~EigSymmWorkspace() { if(evec) gsl_matrix_free(evec); if(W) gsl_eigen_symmv_free(W); }
    /// Assignment
    EigSymmWorkspace& operator=(const EigSymmWorkspace& E) {
        if(_N != E._N) {
            if(evec) gsl_matrix_free(evec);
            if(W) gsl_eigen_symmv_free(W);
            evec = nullptr;
            W = nullptr;
        }
        _N = E._N;
        return *this;
    }

    /// Decompose symmetric (lower-triangle) A -> U D U^T; return eigenvectors in A -> U columns
    void decompSymm(gsl_matrix*& A, gsl_vector* D) {
        if(!evec) evec = gsl_matrix_alloc(_N,_N);
        if(!W) W = gsl_eigen_symmv_alloc(_N);
        gsl_eigen_symmv(A, D, evec, W);
        std::swap(evec,A);
    }

protected:
    size_t _N;
    gsl_matrix* evec = nullptr;             ///< eigenvector storage
    gsl_eigen_symmv_workspace* W = nullptr; ///< symmetrix matrix eigendecomposition workspace
};


//----------------------------------------------------------
/// Re-usable workspace for projecting an N-dimensional ellipsoid into an M-dimensional affine subspace
// formula from Stephen B. Pope, "Algorithms for Ellipsoids," Cornell University Report FDA-08-01 (2008)
class ellipse_affine_projector: public SVDWorkspace {
public:
    /// Constructor
    ellipse_affine_projector(size_t n, size_t m): SVDWorkspace(n),
    M(m), TT(M,n), P(M,M), Mmn(M,n), Mnn(n,n) { }

    /// Set T to vectors along specified axes
    template<typename V>
    void setAxes(const V& a) {
        gsl_matrix_set_zero(TT);
        for(size_t i=0; i<M; i++) TT(i, a[i]) = 1.;
    }

    /// Project from Cholesky form L(lower) to PCA P = U sigma^-1 (or P = U sigma if lsigma = false)
    void projectL(const gsl_matrix* L, bool lsigma = true);

    const size_t M;         ///< input rows
    gsl_matrix_wrapper TT;  ///< T^T [M,N] orthogonal matrix defining affine subspace in rows
    gsl_matrix_wrapper P;   ///< P[M,M] result projection principal axes in columns

protected:
    gsl_matrix_wrapper Mmn; ///< MxN workspace
    gsl_matrix_wrapper Mnn; ///< NxN workspace
};

#endif
