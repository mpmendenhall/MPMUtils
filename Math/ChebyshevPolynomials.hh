/// \file ChebyshevPolynomials.hh Chebyshev Polynomials
// -- Michael P. Mendenhall, 2019

#ifndef CHEBYSHEVPOLYNOMIALS_HH
#define CHEBYSHEVPOLYNOMIALS_HH

#include "Polynomial.hh"
#include <cstdint>

/// Calculate and store Chebyshev Polynomials of the First Kind T_n
// Orthogonal on [-1,1] weighted by 1/sqrt(1-x^2) = 1/sin(theta)
// T_n(cos theta) = cos(n theta)
class Chebyshev_T {
public:
    /// Univariate rational-coeffecient polynomial
    typedef MonovariatePolynomial<int64_t> polynomial_t;

    /// Calculate and return Legendre polynomial P_n(x)
    const polynomial_t& operator()(size_t n);

protected:
    const polynomial_t T0{{{0, 1}}};    ///< T_0(x) = 1
    const polynomial_t T1{{{1, 1}}};    ///< T_1(x) = x
    vector<polynomial_t> Tn{T0,T1};     ///< precalculated T_n polynomials
};

/// Calculate and store Chebyshev Polynomials of the Second Kind U_n
// Orthogonal on [-1,1] weighted by sqrt(1-x^2) = sin(theta)
// U_n(cos theta) = sin((n+1) theta) / sin(theta)
class Chebyshev_U {
public:
    /// Univariate rational-coeffecient polynomial
    typedef MonovariatePolynomial<int64_t> polynomial_t;

    /// Calculate and return Legendre polynomial P_n(x)
    const polynomial_t& operator()(size_t n);

protected:
    const polynomial_t U0{{{0, 1}}};    ///< U_0(x) = 1
    const polynomial_t U1{{{1, 2}}};    ///< U_1(x) = 2x
    vector<polynomial_t> Un{U0,U1};     ///< precalculated U_n polynomials
};

#endif
