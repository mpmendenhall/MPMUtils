/// \file testPolynomial.cc Test of polynomial manipulation

#include "Polynomial.hh"
#include "PolyEval.hh"
#include "PolyFit.hh"
#include "CodeVersion.hh"
#include "NGrid.hh"
#include "BBox.hh"
#include <stdlib.h>
#include <chrono>
#include "Abstract.hh"
using std::chrono::steady_clock;

//typedef float precision_t;
typedef double precision_t;
//typedef long double precision_t;
//typedef __float128 precision_t;
typedef Monomial_t<3,precision_t> Mxyz;
typedef Polynomial_t<3,precision_t> Pxyz;

/// Comparison simple algorithm
template<class P, class T, class Cvec>
void addSimple(const P& p, vector<T>& v, const Cvec& c) {
    if(!v.size()) v.resize(c.size());
    assert(v.size() <= c.size());
    size_t i = 0;
    for(auto& y: v) y += p(c[i++]);
}

int main(int, char**) {
    CodeVersion::display_code_version();

    ////////////////////////
    // construct polynomials

    // 1-variable (integer exponent), float coeffs
    AbstractPolynomial<float,int> AP2;
    AP2[1] = 6;
    AP2[2] = 2;
    AP2[3] = 3;
    std::cout << AP2 << AP2*AP2 << "\n";

    // 2-variable, integer coeffs
    AbstractPolynomial<int,EVec<2>> AP;
    AP[EVec<2>({0,1})] = 6;
    AP[EVec<2>({1,1})] = 2;
    AP[EVec<2>({2,0})] = 3;
    std::cout << AP << AP*AP << "\n";

    std::vector<precision_t> x(3);

    auto p = Pxyz::lowerTriangleTerms(3,1);
    std::cout << "Poly(xyz^3) " << p << "\n";
    std::cout << p << " -> " << double(p(x)) << "\n";

    //assert(p == p.integral(2).derivative(2));

    //auto ip = p.integral(2);
    //auto p2 = reduce(ip,2);
    //std::cout << p2 << "\n";

    /////////////////////////////////////////////////
    // compare time between fast and naive evaluation

    PolyEval<precision_t> PE;
    int ntrials = 10;
    vector<PolyEval<precision_t>::coord_t<3>> vc(1000000/ntrials);
    for(size_t i=0; i<vc.size(); i++) {
        vc[i] = {0.5*i*1e-5, sqrt(i/1e4), i*1.e-10*i};
    }
    vector<precision_t> vp, vp2;

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

    ///////////////////////////
    // generate evaluation grid

    BBox<3,precision_t> BB;
    BB.expand({precision_t(-1),precision_t(-1),precision_t(-1)});
    BB.expand({precision_t(1),precision_t(1),precision_t(1)});
    NGrid<3> NG({5,5,5});
    vc.clear();
    for(auto c: NG) {
        auto x = NG.centerpos(c,BB);
        printf("%i,%i,%i -> %g\t%g\t%g\n", c[0], c[1], c[2], double(x[0]), double(x[1]), double(x[2]));
        vc.push_back(x);
    }

    //////////////////////////////////
    // fit polynomial over grid points

    PolyFit<Pxyz> PF(Pxyz::lowerTriangleTerms(3,3.14));
    std::cout << PF.getPoly() << "\n";
    PF.setX(vc);
    vector<precision_t> y;
    PE.setX(vc);
    PE.evalPolynomial(PF.getPoly(), y);
    auto& PP = PF.solve(y);
    std::cout << PP << "\n" << PF.ssresid() << "\n";

    return EXIT_SUCCESS;
}
