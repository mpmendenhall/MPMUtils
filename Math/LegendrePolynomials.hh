/// \file LegendrePolynomials.hh Legendre Polynomials (orthonormal polynomial basis on [-1,1]) generation
// Michael P. Mendenhall, 2019

#ifndef LEGENDREPOLYNOMIALS_HH
#define LEGENDREPOLYNOMIALS_HH

#include "Rational.hh"
#include "Polynomial.hh"

/// Calculate and store Legendre polynomials
class LegendrePolynomials {
public:
    /// Univariate rational-coeffecient polynomial
    typedef Pol1_t<Rational> polynomial_t;

    /// Calculate and return Legendre polynomial P_n(x)
    const polynomial_t& Legendre_P(size_t n);

protected:
    const polynomial_t P0{{{0, {1,1}}}};        ///< P_0(x) = 1
    const polynomial_t P1{{{1, {1,1}}}};        ///< P_1(x) = x
    vector<polynomial_t> _Legendre_P{P0,P1};    ///< precalculated Legendre polynomials
};

#endif
