/// \file Quadratic.hh Multivariate quadratic polynomial calculations
// Michael P. Mendenhall, 2018

#ifndef QUADRATIC_HH
#define QUADRATIC_HH

#include <stdio.h>
#include <array>
using std::array;
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_matrix.h>

/// Coefficients for multivariate quadratic x^T A x + b.x + c, with factorization to (x-x0)^T A (x-x0) + k
template<size_t N, typename T = double>
class Quadratic {
public:
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

    /// set quadratic term coefficient
    void setCoeff(size_t i, size_t j, T v) {
        if(i < j) std::swap(i,j);
        A[(i*(i-1))/2+j] = v;
        factored = false;
    }
    /// set linear term coefficient
    void setCoeff(size_t i, T v) { b[i] = v; factored = false; }
    /// set constant-order coefficient
    void setCoeff(T v) { k += v-c; c = v; }

    /// add quadratic term coefficient
    void addCoeff(size_t i, size_t j, T v) {
        if(i < j) std::swap(i,j);
        A[(i*(i-1))/2+j] += v;
        factored = false;
    }
    /// add linear term coefficient
    void addCoeff(size_t i, T v) { b[i] += v; factored = false; }
    /// add constant-order coefficient
    void addCoeff(T v) { c += v; k += v; }

    /// evaluate at given point
    template<typename coord>
    T operator()(const coord& v) const {
        T s = c;
        int n = 0;
        s = c;
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
        if(factored) {
             printf("x0 =");
            for(size_t i=0; i<N; i++) printf("\t%g", x0[i]);
            printf(";\tk = %g\n", k);
        }
    }

    /// inplace addition
    void operator+=(const Quadratic& Q) {
        factored = false;
        c += Q.c;
        for(size_t i=0; i<N; i++) b[i] += Q.b[i];
        for(size_t i=0; i<(N*(N+1))/2; i++) A[i] += Q.A[i];
    }

    /// inplace scalar multiplication
    void operator*=(T s) {
        for(auto& v: A) v *= s;
        for(auto& v: b) b *= s;
        k *= s;
        c *= s;
    }

    /// Re-usable allocated matrix working space
    struct Workspace {
        /// Constructor
        Workspace(): M(gsl_matrix_alloc(N,N)), v(gsl_vector_alloc(N)) { }
        /// Destructor
        ~Workspace() { gsl_matrix_free(M); gsl_vector_free(v); }
        gsl_matrix* M;  ///< N*N matrix
        gsl_vector* v;  ///< N-element vector
    };

    /// perform factoring; return center
    const array<T,N>& factor(Workspace& W) {
        if(factored) return x0;

        // solve A x0 = -b/2
        int n = 0;
        for(size_t i=0; i<N; i++) {
            gsl_vector_set(W.v, i, -0.5*b[i]);
            for(size_t j=0; j<=i; j++) gsl_matrix_set(W.M, i, j, (i==j? 1 : 0.5)*A[n++]);
        }
        gsl_linalg_cholesky_decomp1(W.M);
        gsl_linalg_cholesky_svx(W.M, W.v);
        for(size_t i=0; i<N; i++) x0[i] = gsl_vector_get(W.v, i);

        // k = c + b x0 / 2
        k = 0;
        for(size_t i=0; i<N; i++) k += x0[i]*b[i];
        k = c + 0.5*k;
        factored = true;
        return x0;
    }

    /// perform factoring (allocate/deallocate workspace), return center
    const array<T,N>& factor() { if(!factored) { Workspace W; return factor(W); } return x0; }

    /// evaluate terms for linear fit at specified coordinate
    template<typename coord>
    static void evalTerms(const coord& v, array<T, ((N+2)*(N+1))/2>& t) {
        int n = 0;
        for(size_t i=0; i<N; i++) for(size_t j=0; j<=i; j++) t[n++] = v[i]*v[j];
        for(size_t i=0; i<N; i++) t[n++] = v[i];
        t[n] = 1;
    }

protected:
/*
    A coefficients stored in lower-triangular order:
    x  *  x
    y  *  x,y
    z  *  x,y,z
*/
    array<T, (N*(N+1))/2> A;    ///< quadratic form coefficients
    array<T,N> b;               ///< linear coefficients
    array<T,N> x0;              ///< factored centers (extremum location)
    T c = 0;                    ///< unfactored offset
    T k = 0;                    ///< factored offset (extremum value)
    bool factored = false;      ///< whether factored form is calculated
};

#endif
