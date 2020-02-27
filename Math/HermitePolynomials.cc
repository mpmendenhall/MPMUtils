/// \file HermitePolynomials.cc

#include "HermitePolynomials.hh"

const HermitePolynomials::polynomial_t& HermitePolynomials::operator()(size_t n) {
    while(n >= Hn.size())  {
        // constructing H_{m+1}, using recurrence relation
        // H_{m+1} = 2 x H_m - 2 m H_{m-1}
        int m = Hn.size()-1;
        Hn.push_back(H1*Hn[m] + Hn[m-1] * polynomial_t(-2*m));
    }
    return Hn[n];
}
