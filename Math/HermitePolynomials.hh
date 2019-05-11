/// \file HermitePolynomials.hh Hermite Polynomials (orthogonal polynomial basis on [-infty,infty] weighted by e^{-x^2})
// Michael P. Mendenhall, 2019

#ifndef HERMITEPOLYNOMIALS_HH
#define HERMITEPOLYNOMIALS_HH

#include "Polynomial.hh"
#include <cstdint>

/// Calculate and store Hermite polynomials
class HermitePolynomials {
public:
    /// Univariate integer-coeffecient polynomial
    typedef MonovariatePolynomial<int64_t> polynomial_t;

    /// Calculate and return Hermite polynomial H_n(x)
    const polynomial_t& operator()(size_t n);

protected:
    const polynomial_t H0{{{0, 1}}};    ///< H_0(x) = 1
    const polynomial_t H1{{{1, 2}}};    ///< H_1(x) = 2x
    vector<polynomial_t> Hn{H0,H1};     ///< precalculated Hermite polynomials
};

#endif
