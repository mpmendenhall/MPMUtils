/// \file LinalgHelpers.cc
// Michael P. Mendenhall, 2018

#include "LinalgHelpers.hh"
#include <initializer_list>

void rmul_diag(gsl_matrix* M, const gsl_vector* D) {
    for(size_t i=0; i<M->size1; i++)
        for(size_t j=0; j<M->size2; j++)
            gsl_matrix_set(M, i, j, gsl_matrix_get(M, i, j)*gsl_vector_get(D, j));
}

void rdiv_diag(gsl_matrix* M, const gsl_vector* D) {
    for(size_t i=0; i<M->size1; i++)
        for(size_t j=0; j<M->size2; j++)
            gsl_matrix_set(M, i, j, gsl_matrix_get(M, i, j)/gsl_vector_get(D, j));
}

void lmul_diag(gsl_matrix* M, const gsl_vector* D) {
    for(size_t i=0; i<M->size1; i++)
        for(size_t j=0; j<M->size2; j++)
            gsl_matrix_set(M, i, j, gsl_matrix_get(M, i, j)*gsl_vector_get(D, i));
}

void invert_colnorms(gsl_matrix* M) {
    for(size_t i=0; i<M->size2; i++) {
        double cc = 0;
        for(size_t j=0; j<M->size1; j++) cc += gsl_matrix_get(M,j,i)*gsl_matrix_get(M,j,i);
        for(size_t j=0; j<M->size1; j++) gsl_matrix_set(M, j, i, gsl_matrix_get(M,j,i)/cc);
    }
}

void zeroTriangle(CBLAS_UPLO_t uplo, gsl_matrix* A) {
    if(uplo == CblasLower) {
        for(size_t i=1; i<A->size1; i++)
            for(size_t j=0; j<i; j++)
                gsl_matrix_set(A,i,j,0.);
    } else {
        for(size_t i=0; i<A->size1; i++)
            for(size_t j=i+1; j<A->size2; j++)
                gsl_matrix_set(A,i,j,0.);
    }
}

void fillSymmetric(CBLAS_UPLO_t uplo, gsl_matrix* A) {
        if(uplo == CblasLower) {
        for(size_t i=1; i<A->size1; i++)
            for(size_t j=0; j<i; j++)
                gsl_matrix_set(A,i,j, gsl_matrix_get(A,j,i));
    } else {
        for(size_t i=0; i<A->size1; i++)
            for(size_t j=i+1; j<A->size2; j++)
                gsl_matrix_set(A,i,j, gsl_matrix_get(A,j,i));
    }
}

void resize(gsl_vector*& g, size_t n) {
    if(g && g->size != n) { gsl_vector_free(g); g = nullptr; }
    if(!g) g = gsl_vector_alloc(n);
}

/////////////////////

SVDWorkspace::SVDWorkspace(size_t n, size_t m): N(n), M(m),
V(gsl_matrix_alloc(N,N)), S(gsl_vector_alloc(N)), w(gsl_vector_alloc(N)) { }

SVDWorkspace::~SVDWorkspace() {
    gsl_matrix_free(V);
    gsl_vector_free(S);
    gsl_vector_free(w);
}

/////////////////////////
// S. B. Pope's notation:
//
// U = unitary principal axis directions
// S (\Sigma) = diagonal matrix, 1/S_ii = length of principal axis i
// A = U S^2 U^T = L L^T (Cholesky form, L lower-triangular)
// w = S U^T x ; x = U/S w
// origin-centered ellipse surface defined by:
// E = {x: x A x = 1}                              (quadratic coeffs form)
// E = {x: x = U/S w,  |w|=1} ; {x: |S U^T x| = 1} (PCA form)
// E = {x: x = L^-T u, |u|=1} ; {x: |L^T x| = 1}   (Cholesky form)

ellipse_affine_projector::ellipse_affine_projector(size_t n, size_t m): SVDWorkspace(n,m),
TT(gsl_matrix_calloc(M,N)), P(gsl_matrix_alloc(M,M)), Mmn(gsl_matrix_alloc(M,N)), Mnn(gsl_matrix_alloc(N,N)) { }

ellipse_affine_projector::~ellipse_affine_projector() { for(auto m: {TT,P,Mmn,Mnn}) gsl_matrix_free(m); }

void ellipse_affine_projector::projectL(const gsl_matrix* L, bool lsigma) {
    // Mmn = T^T (M,N) L^-T (N,N)
    gsl_matrix_memcpy(Mmn, TT);
    gsl_blas_dtrsm(CblasRight, CblasLower, CblasTrans, CblasNonUnit, 1., L, Mmn);
    // embed Mmn in (rank-deficient) NxN matrix
    gsl_matrix_set_zero(Mnn);
    for(size_t i=0; i<M; i++)
        for(size_t j=0; j<N; j++)
            gsl_matrix_set(Mnn, i, j, gsl_matrix_get(Mmn, i, j));
    // SVD Mnn = U S V^T, Mnn -> U ; S ~ principal axis lengths
    SVD(Mnn);
    // normalization: U/S = Lt Q; want U S for principal axes
    // Copy projected component to output P = U S
    for(size_t i=0; i<M; i++) {
        for(size_t j=0; j<M; j++) {
            double s = gsl_vector_get(S, j);
            double m = gsl_matrix_get(Mnn, i, j);
            gsl_matrix_set(P, i, j, lsigma? m*s : m/s);
        }
    }
}

/////////////////

void displayV(const gsl_vector* v) {
    printf("< ");
    for(size_t i=0; i<v->size; i++) printf("%g ", gsl_vector_get(v,i));
    printf(">\n");
}

void displayM(const gsl_matrix* M) {
    printf("---- matrix %zu x %zu ----\n", M->size1, M->size2);
    for(size_t i=0; i<M->size1; i++) {
        for(size_t j=0; j<M->size2; j++) printf("\t%g", gsl_matrix_get(M,i,j));
        printf("\n");
    }
}
