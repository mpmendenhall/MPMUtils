/// \file Quadratic.hh Multivariate quadratic polynomial calculations
// Michael P. Mendenhall, 2018

#ifndef QUADRATIC_HH
#define QUADRATIC_HH

#include <stdio.h>
#include <cmath>
#include "LinalgHelpers.hh"

/// Coefficients for multivariate quadratic x^T A x + b.x + c
class Quadratic {
public:
    /// total number of terms
    static size_t nterms(const size_t n) { return ((n+2)*(n+1))/2; }

    /// default constructor
    Quadratic(size_t n): N(n), A((N*(N+1))/2), b(N) { }

    /// number of dimensions
    const size_t N;
    /// coordinate x,y,z...
    typedef vector<double> coord_t;
    /// individual evaluated terms at a coordinate
    typedef vector<double> terms_t;

    /// unpack from coefficients array
    template<typename coeffs>
    void setCoeffs(const coeffs& v) {
        int n = 0;
        for(auto& x: A) x = v[n++];
        for(auto& x: b) x = v[n++];
        c = v[n];
    }

    /// unpack to coefficients array
    template<typename coeffs>
    void getCoeffs(coeffs& v) const {
        int n = 0;
        for(auto& x: A) v[n++] = x;
        for(auto& x: b) v[n++] = x;
        v[n] = c;
    }

/*
    A coefficients stored in lower-triangular order:
    x  *  x
    y  *  x,y
    z  *  x,y,z
*/
    vector<double> A;   ///< quadratic form coefficients
    coord_t b;          ///< linear coefficients
    double c = 0;       ///< x=0 value

    /// set quadratic term coefficient
    void setCoeff(size_t i, size_t j, double v) {
        if(i < j) std::swap(i,j);
        A[(i*(i-1))/2+j] = v;
    }
    /// add quadratic term coefficient
    void addCoeff(size_t i, size_t j, double v) {
        if(i < j) std::swap(i,j);
        A[(i*(i-1))/2+j] += v;
    }

    /// evaluate quadratic form component only
    template<typename coord>
    double xTAx(const coord& x) const {
        double s = 0;
        int n = 0;
        for(size_t i=0; i<N; i++)
            for(size_t j=0; j<=i; j++)
                s += x[i]*x[j]*A[n++];
        return s;
    }

    /// evaluate at given point
    template<typename coord>
    double operator()(const coord& v) const {
        double s = c;
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
    void operator*=(double s) {
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
        const size_t N = v.size();
        t.resize(nterms(N));
        int n = 0;
        for(size_t i=0; i<N; i++) for(size_t j=0; j<=i; j++) t[n++] = v[i]*v[j];
        for(size_t i=0; i<N; i++) t[n++] = v[i];
        t[n] = 1;
    }
};



/// Cholesky decomposition x^T A x + b^T x + c --> (x-x0)^T L L^T (x-x0) + k
class QuadraticCholesky {
public:
    /// Constructor
    QuadraticCholesky(size_t n): N(n), x0(N), L(N,N), M(N,N), v(N) { }

    /// perform Cholesky decomposition of quadratic form
    template<class Quad>
    void calcCholesky(const Quad& Q) {
        Q.fillA(L);
        //gsl_linalg_cholesky_decomp1(L); // newer GSL only
        gsl_linalg_cholesky_decomp(L);
    }

    /// perform decomposition; solve A x0 = -b/2
    template<class Quad>
    void decompose(const Quad& Q) {
        calcCholesky(Q);
        findCenter(Q.b, Q.c);
    }

    /// calculate A = L L^T
    void getA(gsl_matrix* A) const { gsl_blas_dsyrk(CblasLower, CblasNoTrans, 1.0, L, 0., A); }

    /// multiply Q.A = L L^T
    template<class Quad>
    void fillA(Quad& Q) {
        getA(M);
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

    /// solve for b, c -> x0, k (assuming L in Cholesky form)
    template<class V>
    void findCenter(const V& b, double c = 0) {
        for(size_t i=0; i<N; i++) gsl_vector_set(v, i, -0.5*b[i]);
        gsl_linalg_cholesky_svx(L, v);
        for(size_t i=0; i<N; i++) x0[i] = gsl_vector_get(v, i);
        // k = c + b x0 / 2
        k = 0;
        for(size_t i=0; i<N; i++) k += x0[i]*b[i];
        k = c + 0.5*k;
    }

    const size_t N;         ///< number of dimensions
    vector<double> x0;      ///< extremum position
    double k = 0;           ///< extremum value
    gsl_matrix_wrapper L;   ///< lower-triangular NxN Cholesky decomposition L L^T = A

protected:
    gsl_matrix_wrapper M;   ///< NxN workspace
    gsl_vector_wrapper v;   ///< N-element Cholesky solution vector
};

#ifdef GSL_25

/// Modified Cholesky decomposition for non-positive-definite matrices
template<size_t N, typename T = double>
class QuadraticMCholesky: public QuadraticCholesky<N,T> {
public:
    /// Constructor
    QuadraticMCholesky(): E(N), P(gsl_permutation_alloc(N)) { }
    /// Destructor
    ~QuadraticMCholesky() { gsl_permutation_free(P); }

    /// calculate modified Cholesky decomposition
    void calcMCholesky(const Quadratic& Q) {
        Q.fillA(this->L);
        gsl_linalg_mcholesky_decomp(this->L, P, E);
    }

    /// perform decomposition; solve A x0 = -b/2
    virtual void decompose(const Quadratic& Q) override {
        calcMCholesky(Q);
        for(size_t i=0; i<N; i++) gsl_vector_set(this->v, i, -0.5*Q.b[i]);
        gsl_linalg_mcholesky_svx(this->L, P, this->v);
        this->unpack(Q);
    }

    gsl_vector_wrapper E;   ///< tweak for positive-definiteness
    gsl_permutation* P;     ///< pivoting permutation
};

#endif

/// Principal axes of ellipse defined by quadratic
class QuadraticPCA: protected EigSymmWorkspace {
public:
    /// Constructor
    QuadraticPCA(size_t N): EigSymmWorkspace(N), USi(N,N), S2(N), Si(N) { }

    /// perform decomposition
    template<class Quad>
    void decompose(const Quad& Q, bool doMul = true) {
        Q.fillA(USi);
        decompSymm(USi, S2); // A -> U S^2 U^T
        for(size_t i=0; i<_N; i++) gsl_vector_set(Si, i, 1./sqrt(gsl_vector_get(S2,i)));
        if(doMul) rmul_diag(USi, Si);
    }

    gsl_matrix_wrapper USi; ///< ellipse principal axes in columns
    gsl_vector_wrapper S2;  ///< axes 1/sigma^2
    gsl_vector_wrapper Si;  ///< axes 1*sigma
};

/// Workspace for calculating ellipsoid covering or covered by two concentric ellipsoids
class CoveringEllipse {
public:
    /// Constructor
    CoveringEllipse(size_t n): N(n), E1(N), E2(N), EC(N), SVD(N), L2P(N,N) { }

    const size_t N; ///< number of dimensions

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
        gsl_linalg_cholesky_decomp(EC.L);
        zeroTriangle(CblasUpper, EC.L);
        // L = L1 L'
        gsl_blas_dtrmm(CblasLeft, CblasLower, CblasNoTrans, CblasNonUnit, 1.0, E1.L, EC.L);
    }

    QuadraticCholesky E1;
    QuadraticCholesky E2;
    QuadraticCholesky EC;

protected:
    SVDWorkspace SVD;
    gsl_matrix_wrapper L2P;
};

#endif
