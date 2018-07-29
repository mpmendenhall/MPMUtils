/// \file testPolynomial.cc Test of polynomial manipulation

#include "Polynomial.hh"
#include "PolyEval.hh"
#include <stdlib.h>
#include <chrono>
using std::chrono::steady_clock;

typedef Monomial_t<3> Mxyz;
typedef Polynomial_t<3> Pxyz;

/// Comparison simple algorithm
template<class P, class T, class Cvec>
void addSimple(const P& p, vector<T>& v, const Cvec& c) {
    if(!v.size()) v.resize(c.size());
    assert(v.size() <= c.size());
    size_t i = 0;
    for(auto& y: v) y += p(c[i++]);
}

int main(int, char**) {

    std::vector<double> x(3);

    Mxyz m(5,{3,2,1});
    m += m;
    std::cout << m << "\n";

    auto p = Pxyz::lowerTriangleTerms(4);
    std::cout << p << "\n";
    p.algebraicForm(std::cout,true) << "\n";
    p += p;
    p *= p;
    p += m;
    p = p.integral(1,0,2);
    std::cout << p << " -> " << p(x) << "\n";

    assert(p == p.integral(2).derivative(2));

    auto ip = p.integral(2);
    auto p2 = reduce(ip,2);
    std::cout << p2 << "\n";

    PolyEval<> PE;
    int ntrials = 10;
    vector<PolyEval<>::coord_t<3>> vc(1000000/ntrials);
    for(size_t i=0; i<vc.size(); i++) vc[i] = {0.5*i};
    vector<double> vp, vp2;

    auto t0 = steady_clock::now();

    for(int i=0; i<ntrials; i++) {
        PE.setX(vc);
        PE.addPolynomial(p, vp);
    }

    auto t1 = steady_clock::now();

    for(int i=0; i<ntrials; i++) addSimple(p, vp2, vc);

    auto t2 = steady_clock::now();

    printf("dt1 = %g; dt2 = %g\n",
           std::chrono::duration<double>(t1-t0).count(),
           std::chrono::duration<double>(t2-t1).count());

    double dmax =  0;
    for(size_t i=0; i<vp.size(); i++) {
        //printf("%g\t%g\n", vp[i], vp2[i]);
        dmax = std::max(dmax, fabs((vp[i]-vp2[i])/vp[i]));
    }
    printf("dmax %g\n", dmax);

    return EXIT_SUCCESS;
}
