/// \file testPolynomial.cc Test of polynomial manipulation

#include "Polynomial.hh"
#include "PolyEval.hh"
#include <stdlib.h>
#include <chrono>
using std::chrono::steady_clock;

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

    PolyEval<> PE;
    vector<PolyEval<>::coord_t<3>> vc(1000000, {1.});
    vector<double> vp(vc.size());
    auto vp2 = vp;

    auto t0 = steady_clock::now();

    PE.setX(vc);
    PE.addPolynomial(p, vp);

    auto t1 = steady_clock::now();

    PE.addSimple(p, vp2, vc);

    auto t2 = steady_clock::now();

    printf("dt1 = %g; dt2 = %g\n",
           std::chrono::duration<double>(t1-t0).count(),
           std::chrono::duration<double>(t2-t1).count());

    return EXIT_SUCCESS;
}
