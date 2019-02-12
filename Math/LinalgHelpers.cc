/// \file LinalgHelpers.cc
// Michael P. Mendenhall, 2018

#include "LinalgHelpers.hh"

gsl_matrix_wrapper& gsl_matrix_wrapper::operator=(const gsl_matrix_wrapper& w) {
    if(this == &w) return *this;
    if(M && w.M && w->size1 == M->size1 && w->size2 == M->size2) {
        gsl_matrix_memcpy(M, w);
        return *this;
    }
    if(M) gsl_matrix_free(M);
    if(w.M) {
        M = gsl_matrix_alloc(w->size1, w->size2);
        gsl_matrix_memcpy(M, w);
    } else M = nullptr;
    return *this;
}

gsl_matrix_wrapper& gsl_matrix_wrapper::operator=(gsl_matrix_wrapper&& other) {
    if(this == &other) return *this;
    if(M) gsl_matrix_free(M);
    M = other.M;
    other.M = nullptr;
    return *this;
}

gsl_vector_wrapper& gsl_vector_wrapper::operator=(const gsl_vector_wrapper& w) {
    if(this == &w) return *this;
    if(v && w.v && w->size == v->size) {
        gsl_vector_memcpy(v, w);
        return *this;
    }
    if(v) gsl_vector_free(v);
    if(w.v) {
        v = gsl_vector_alloc(w->size);
        gsl_vector_memcpy(v, w);
    } else v = nullptr;
    return *this;
}

gsl_vector_wrapper& gsl_vector_wrapper::operator=(gsl_vector_wrapper&& other) {
    if(this == &other) return *this;
    if(v) gsl_vector_free(v);
    v = other.v;
    other.v = nullptr;
    return *this;
}

std::ostream& operator<< (std::ostream &o, const gsl_matrix_wrapper& M) {
    if(!(gsl_matrix*)M) {
        o << "0\t0\n";
        return o;
    }
    o << M->size1 << "\t" << M->size2 << "\n";
    for(size_t r=0; r<M->size1; r++) {
        for(size_t c=0; c<M->size2; c++) o << "\t" << M(r,c);
        o << "\n";
    }
    return o;
}

std::istream& operator>> (std::istream &i, gsl_matrix_wrapper& M) {
    size_t m = 0, n = 0;
    i >> m >> n;
    M = gsl_matrix_wrapper(m,n,false);
    for(size_t r=0; r<M->size1; r++)
        for(size_t c=0; c<M->size2; c++)
            i >> M(r,c);
    return i;
}

std::ostream& operator<< (std::ostream &o, const gsl_vector_wrapper& v) {
    if(!(gsl_vector*)v) {
        o << "0\n";
        return o;
    }
    o << v->size << "\n";
    for(size_t i=0; i<v->size; i++) o << "\t" << v(i);
    o << "\n";
    return o;
}

std::istream& operator>> (std::istream &i, gsl_vector_wrapper& v) {
    size_t n = 0;
    i >> n;
    v = gsl_vector_wrapper(n,false);
    for(size_t j=0; j<n; j++) i >> v(j);
    return i;
}

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

void ellipse_affine_projector::projectL(const gsl_matrix* L, bool lsigma) {
    if(!M) return; // project to zero-dimensional subspace

    // Mmn = T^T (M,N) L^-T (N,N)
    gsl_matrix_memcpy(Mmn, TT);
    gsl_blas_dtrsm(CblasRight, CblasLower, CblasTrans, CblasNonUnit, 1., L, Mmn);
    // embed Mmn in (rank-deficient) NxN matrix
    gsl_matrix_set_zero(Mnn);
    for(size_t i=0; i<Mmn->size1; i++)
        for(size_t j=0; j<Mmn->size2; j++)
            Mnn(i, j) = Mmn(i, j);
    // SVD Mnn = U S V^T, Mnn -> U ; S ~ principal axis lengths
    SVD(Mnn);
    // normalization: U/S = Lt Q; want U S for principal axes
    // Copy projected component to output P = U S
    for(size_t i=0; i<M; i++) {
        for(size_t j=0; j<M; j++) {
            double s = S(j);
            double m = Mnn(i, j);
            P(i, j) = lsigma? m*s : m/s;
        }
    }
}

/////////////////

void displayV(const gsl_vector* v) {
    printf("< ");
    if(v) for(size_t i=0; i<v->size; i++) printf("%g ", gsl_vector_get(v,i));
    printf(">\n");
}

void displayM(const gsl_matrix* M) {
    if(!M) { printf("---- matrix 0 x 0 ----\n"); return; }
    printf("---- matrix %zu x %zu ----\n", M->size1, M->size2);
    for(size_t i=0; i<M->size1; i++) {
        for(size_t j=0; j<M->size2; j++) printf("\t%g", gsl_matrix_get(M,i,j));
        printf("\n");
    }
}
