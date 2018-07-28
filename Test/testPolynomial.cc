/// \file testPolynomial.cc Test of polynomial manipulation

#include "Polynomial.hh"
#include <stdlib.h>

typedef Monomial_t<3> Mxyz;
typedef Polynomial_t<3> Pxyz;

int main(int, char**) {

    std::vector<double> x(3);

    Mxyz m(5,{3,2,1});
    m += m;
    std::cout << m << "\n";

    auto p = Pxyz::lowerTriangleTerms(3);
    std::cout << p << "\n";
    p.algebraicForm(std::cout,true) << "\n";
    p += p;
    p *= p;
    p += m;
    p = p.integral(1,0,2);
    std::cout << p << " -> " << p(x) << "\n";

    p.prune();
    p.recentered(x);

    assert(p == p.integral(2).derivative(2));

    auto ip = p.integral(2);
    auto p2 = reduce(ip,2);
    std::cout << p2 << "\n";

    return EXIT_SUCCESS;
}
