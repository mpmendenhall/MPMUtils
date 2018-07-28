/// \file testPolynomial.cc Test of polynomial manipulation

#include "Polynomial.hh"
#include <array>
using std::array;
#include <stdlib.h>

typedef Monomial<array<int,3>, float> Mxyz;
typedef Polynomial<Mxyz> Pxyz;

int main(int, char**) {

    vector<double> x(3);

    Mxyz m(5,{3,2,1});
    m += m;
    std::cout << m << "\n";

    Pxyz p({5,{1,2,3}});
    p += Pxyz(1);
    std::cout << p << "\n";
    p += p;
    p *= p;
    p += m;
    std::cout << p << p(x) << "\n";

    p.prune();
    p.recentered(x);

    return EXIT_SUCCESS;
}
