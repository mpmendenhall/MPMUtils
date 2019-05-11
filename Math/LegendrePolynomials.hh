/// \file LegendrePolynomials.hh Legendre Polynomials (orthonogonal polynomial basis on [-1,1] with weight 1)
// Michael P. Mendenhall, 2019

#ifndef LEGENDREPOLYNOMIALS_HH
#define LEGENDREPOLYNOMIALS_HH

#include "Rational.hh"
#include "Polynomial.hh"

/// Calculate and store Legendre polynomials
class LegendrePolynomials {
public:
    /// Univariate rational-coeffecient polynomial
    typedef MonovariatePolynomial<Rational> polynomial_t;

    /// Calculate and return Legendre polynomial P_n(x)
    const polynomial_t& operator()(size_t n);

protected:
    const polynomial_t P0{{{0, {1,1}}}};    ///< P_0(x) = 1
    const polynomial_t P1{{{1, {1,1}}}};    ///< P_1(x) = x
    vector<polynomial_t> Pn{P0,P1};         ///< precalculated Legendre polynomials
};

#endif
