/// \file LegendrePolynomials.cc

#include "LegendrePolynomials.hh"

const LegendrePolynomials::polynomial_t& LegendrePolynomials::Legendre_P(size_t n) {
    while(n >= _Legendre_P.size())  {
        // constructing P_{m+1}, using recurrence relation
        // (m+1) P_{m+1} - (2m+1) x P_m + m P_{m-1} = 0
        int m = _Legendre_P.size()-1;
        _Legendre_P.push_back(P1*_Legendre_P[m]*Rational(2*m+1, m+1) + _Legendre_P[m-1] * Rational(-m, m+1));
    }
    return _Legendre_P[n];
}
