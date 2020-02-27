/// \file LaguerrePolynomials.cc

#include "LaguerrePolynomials.hh"

const LaguerrePolynomials::polynomial_t& LaguerrePolynomials::operator()(size_t n) {
    while(n >= Ln.size())  {
        // constructing L_{m+1}, using recurrence relation
        // (m+1) L_{m+1} = (2m+1-x) L_m - m L_{m-1}
        int m = Ln.size()-1;
        Ln.push_back(polynomial_t({{0, {2*m+1, m+1}}, {1, {-1, m+1}}})*Ln[m] + Ln[m-1] * polynomial_t(Rational(-m, m+1)));
    }
    return Ln[n];
}
