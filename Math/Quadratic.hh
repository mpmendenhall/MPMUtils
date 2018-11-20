/// \file Quadratic.hh Multivariate quadratic polynomial calculations
// Michael P. Mendenhall, 2018

#ifndef QUADRATIC_HH
#define QUADRATIC_HH

#include <stdio.h>
#include <array>
using std::array;
#include "LinalgHelpers.hh"

/// Coefficients for multivariate quadratic x^T A x + b.x + c
template<size_t N, typename T = double>
class Quadratic {
public:
    /// total number of terms in A, b, c
    static constexpr size_t NTERMS = ((N+2)*(N+1))/2;
    /// coordinate x,y,z...
    typedef array<T, N> coord_t;
    /// individual evaluated terms at a coordinate
    typedef array<T, NTERMS> terms_t;

    /// default constructor
    Quadratic(): A(), b() { }

    /// constructor from coefficients array
    template<typename coeffs>
    Quadratic(const coeffs& v) {
        int n = 0;
        for(auto& x: A) x = v[n++];
        for(auto& x: b) x = v[n++];
        c = v[n];
    }

/*
    A coefficients stored in lower-triangular order:
    x  *  x
    y  *  x,y
    z  *  x,y,z
*/
    array<T, (N*(N+1))/2> A;    ///< quadratic form coefficients
    coord_t b;                  ///< linear coefficients
    T c = 0;                    ///< x=0 value

    /// set quadratic term coefficient
    void setCoeff(size_t i, size_t j, T v) {
        if(i < j) std::swap(i,j);
        A[(i*(i-1))/2+j] = v;
    }
    /// add quadratic term coefficient
    void addCoeff(size_t i, size_t j, T v) {
        if(i < j) std::swap(i,j);
        A[(i*(i-1))/2+j] += v;
    }

    /// evaluate quadratic form component only
    template<typename coord>
    T xTAx(const coord& x) const {
        T s = 0;
        int n = 0;
        for(size_t i=0; i<N; i++)
            for(size_t j=0; j<=i; j++)
                s += x[i]*x[j]*A[n++];
        return s;
    }

    /// evaluate at given point
    template<typename coord>
    T operator()(const coord& v) const {
        T s = c;
        int n = 0;
        for(size_t i=0; i<N; i++) {
            s += b[i]*v[i];
            for(size_t j=0; j<=i; j++) s += v[i]*v[j]*A[n++];
        }
        return s;
    }

    /// display contents
    void display() const {
        int n = 0;
        for(size_t i=0; i<N; i++) {
            for(size_t j=0; j<=i; j++) printf("\t%g", A[n++]);
            printf("\n");
        }
        printf("b =");
        for(size_t i=0; i<N; i++) printf("\t%g", b[i]);
        printf(";\tc = %g\n", c);

    }

    /// inplace addition
    void operator+=(const Quadratic& Q) {
        c += Q.c;
        for(size_t i=0; i<N; i++) b[i] += Q.b[i];
        for(size_t i=0; i<(N*(N+1))/2; i++) A[i] += Q.A[i];
    }

    /// inplace scalar multiplication
    void operator*=(T s) {
        for(auto& v: A) v *= s;
        for(auto& v: b) v *= s;
        c *= s;
    }

    /// fill lower triangle of NxN matrix A
    void fillA(gsl_matrix* MA) const {
        int n = 0;
        for(size_t i=0; i<N; i++)
            for(size_t j=0; j<=i; j++)
                gsl_matrix_set(MA, i, j, (i==j? 1 : 0.5)*A[n++]);
    }

    /// evaluate terms for linear fit at specified coordinate
    template<typename coord>
    static void evalTerms(const coord& v, terms_t& t) {
        int n = 0;
        for(size_t i=0; i<N; i++) for(size_t j=0; j<=i; j++) t[n++] = v[i]*v[j];
        for(size_t i=0; i<N; i++) t[n++] = v[i];
        t[n] = 1;
    }
};


/// Cholesky decomposition of quadratic to (x-x0)^T L L^T (x-x0) + k
template<size_t N, typename T = double>
class QuadraticCholesky {
public:
    /// Constructor
    QuadraticCholesky(): x0(), L(gsl_matrix_alloc(N,N)), M(gsl_matrix_alloc(N,N)), v(gsl_vector_alloc(N)) { }
    /// Destructor
    ~QuadraticCholesky() { gsl_matrix_free(L); gsl_matrix_free(M); gsl_vector_free(v); }

    typedef array<T, N> coord_t;

    /// perform Cholesky decomposition of quadratic form
    void calcCholesky(const Quadratic<N,T>& Q) {
        Q.fillA(L);
        gsl_linalg_cholesky_decomp1(L);
    }

    /// perform decomposition; solve A x0 = -b/2
    void decompose(const Quadratic<N,T>& Q) {
        calcCholesky(Q);
        for(size_t i=0; i<N; i++) gsl_vector_set(v, i, -0.5*Q.b[i]);
        gsl_linalg_cholesky_svx(L, v);
        for(size_t i=0; i<N; i++) x0[i] = gsl_vector_get(v, i);

        // k = c + b x0 / 2
        k = 0;
        for(size_t i=0; i<N; i++) k += x0[i]*Q.b[i];
        k = Q.c + 0.5*k;
    }

    /// multiply Q.A = L L^T
    void fillA(Quadratic<N,T>& Q) {
        gsl_blas_dsyrk(CblasLower, CblasNoTrans, 1.0, L, 0., M);
        int n = 0;
        for(size_t i=0; i<N; i++)
            for(size_t j=0; j<=i; j++)
                Q.A[n++] = gsl_matrix_get(M, i, j) * (i==j? 1 : 2);
    }

    /// projection length onto unit direction vector
    template<class V>
    double projLength(const V& d) {
        for(size_t i=0; i<N; i++) gsl_vector_set(v, i, d[i]);
        gsl_blas_dtrsv(CblasLower, CblasNoTrans, CblasNonUnit, L, v);
        return gsl_blas_dnrm2(v);
    }

    void display() const {
        printf("x0 =");
        for(size_t i=0; i<N; i++) printf("\t%g", x0[i]);
        printf(";\tk = %g\n", k);
    }

    coord_t x0;     ///< extremum position
    T k = 0;        ///< extremum value
    gsl_matrix* L;  ///< lower-triangular NxN Cholesky decomposition L L^T = A

protected:
    gsl_matrix *M;  ///< NxN workspace
    gsl_vector* v;  ///< N-element vector
};

/// Principal axes of ellipse defined by quadratic
template<size_t N, typename T = double>
class QuadraticPCA: protected EigSymmWorkspace {
public:
    /// Constructor
    QuadraticPCA(): EigSymmWorkspace(N), USi(gsl_matrix_alloc(N,N)), S2(gsl_vector_alloc(N)), Si(gsl_vector_alloc(N)) { }
    /// Destructor
    ~QuadraticPCA() { gsl_matrix_free(USi); gsl_vector_free(S2); gsl_vector_free(Si); }

    /// perform decomposition
    void decompose(const Quadratic<N,T>& Q) {
        Q.fillA(USi);
        decompSymm(USi, S2); // A -> U S^2 U^T
        for(size_t i=0; i<N; i++) gsl_vector_set(Si, i, 1./sqrt(gsl_vector_get(S2,i)));

        rmul_diag(USi, Si);
    }

    gsl_matrix* USi;        ///< ellipse principal axes in columns
    gsl_vector* S2;         ///< axes 1/sigma^2
    gsl_vector* Si;         ///< axes 1*sigma
};

/// Workspace for calculating ellipsoid covering or covered by two concentric ellipsoids
template<size_t N, typename T = double>
class CoveringEllipse {
public:
    /// Constructor
    CoveringEllipse(): SVD(N,N), L2P(gsl_matrix_alloc(N,N)) { }
    /// Destructor
    ~CoveringEllipse() { gsl_matrix_free(L2P); }

    /// Calculate covering ellipse EC from E1, E2
    void calcCovering(bool cover = true) {
        zeroTriangle(CblasUpper, E1.L);
        zeroTriangle(CblasUpper, E2.L);

        // L2' = L1^-1 L2
        gsl_matrix_memcpy(L2P, E2.L);
        gsl_blas_dtrsm(CblasLeft, CblasLower, CblasNoTrans, CblasNonUnit, 1., E1.L, L2P);
        // L2' -> U Sigma V^T
        SVD.SVD(L2P);
        // Sigma -> ~S
        for(size_t i=0; i<N; i++) {
            double s = gsl_vector_get(SVD.S, i);
            gsl_vector_set(SVD.S, i, cover? std::min(s,1.) : std::max(s,1.));
        }
        // EC.L = L' L'^T = U ~S^2 U^T
        rmul_diag(L2P, SVD.S);
        gsl_blas_dsyrk(CblasLower, CblasNoTrans, 1.0, L2P, 0., EC.L);
        // solve for L'
        gsl_linalg_cholesky_decomp1(EC.L);
        zeroTriangle(CblasUpper, EC.L);
        // L = L1 L'
        gsl_blas_dtrmm(CblasLeft, CblasLower, CblasNoTrans, CblasNonUnit, 1.0, E1.L, EC.L);
    }

    QuadraticCholesky<N,T> E1;
    QuadraticCholesky<N,T> E2;
    QuadraticCholesky<N,T> EC;

protected:
    SVDWorkspace SVD;
    gsl_matrix* L2P;
};

#endif
