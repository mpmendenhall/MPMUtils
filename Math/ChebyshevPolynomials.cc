/// @file ChebyshevPolynomials.cc

#include "ChebyshevPolynomials.hh"

const Chebyshev_T::polynomial_t& Chebyshev_T::operator()(size_t n) {
    while(n >= Tn.size())  {
        // constructing T_{m+1}, using recurrence relation
        // T_{m+1} = 2 x T_m - T_{m-1}
        int m = Tn.size()-1;
        Tn.push_back(T1*Tn[m]*polynomial_t(2) + Tn[m-1]*polynomial_t(-1));
    }
    return Tn[n];
}

const Chebyshev_U::polynomial_t& Chebyshev_U::operator()(size_t n) {
    while(n >= Un.size())  {
        // constructing U_{m+1}, using recurrence relation
        // U_{m+1} = 2 x U_m - U_{m-1}
        int m = Un.size()-1;
        Un.push_back(U1*Un[m] + Un[m-1]*polynomial_t(-1));
    }
    return Un[n];
}
