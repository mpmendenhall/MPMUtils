/// @file LaguerrePolynomials.hh Laguerre Polynomials (orthonogonal basis on [0,infty] with weight e^{-x})
// -- Michael P. Mendenhall, 2019

#ifndef LAGUERREPOLYNOMIALS_HH
#define LAGUERREPOLYNOMIALS_HH

#include "Rational.hh"
#include "Polynomial.hh"

/// Calculate and store Legendre polynomials
class LaguerrePolynomials {
public:
    /// Univariate rational-coeffecient polynomial
    typedef MonovariatePolynomial<Rational> polynomial_t;

    /// Calculate and return Laguerre polynomial L_n(x)
    const polynomial_t& operator()(size_t n);

protected:
    const polynomial_t L0{{ {0, {1,1}} }};              ///< L_0(x) = 1
    const polynomial_t L1{{ {0, {1,1}}, {1, {-1,1}} }}; ///< L_1(x) = 1-x
    vector<polynomial_t> Ln{L0,L1};         ///< precalculated Laguerre polynomials
};

#endif
