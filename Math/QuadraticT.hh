/// \file QuadraticT.hh templatized fixed-size version of Quadratic

#ifndef QUADRATICT_HH
#define QUADRATICT_HH

#include <array>
using std::array;

/// Coefficients for multivariate quadratic x^T A x + b.x + c
template<size_t N, typename T = double>
class QuadraticT {
public:
    /// total number of terms in A, b, c
    static constexpr size_t NTERMS = ((N+2)*(N+1))/2;
    /// coordinate x,y,z...
    typedef array<T, N> coord_t;
    /// individual evaluated terms at a coordinate
    typedef array<T, NTERMS> terms_t;

    /// default constructor
    QuadraticT(): A(), b() { }

    /// constructor from coefficients array
    template<typename coeffs>
    explicit QuadraticT(const coeffs& v) {
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
    void operator+=(const QuadraticT& Q) {
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

#endif

