/// \file LegendrePolynomials.cc

#include "LegendrePolynomials.hh"

const LegendrePolynomials::polynomial_t& LegendrePolynomials::operator()(size_t n) {
    while(n >= Pn.size())  {
        // constructing P_{m+1}, using recurrence relation
        // (m+1) P_{m+1} - (2m+1) x P_m + m P_{m-1} = 0
        int m = Pn.size()-1;
        Pn.push_back(P1*Pn[m]*Rational(2*m+1, m+1) + Pn[m-1] * Rational(-m, m+1));
    }
    return Pn[n];
}
