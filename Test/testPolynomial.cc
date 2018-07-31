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
using std::cout;

//typedef float precision_t;
typedef double precision_t;
//typedef long double precision_t;
//typedef __float128 precision_t;

/// Comparison simple algorithm
template<class P, class T, class Cvec>
void addSimple(const P& p, vector<T>& v, const Cvec& c) {
    if(!v.size()) v.resize(c.size());

    size_t i = 0;
    for(auto& y: v) {
        for(auto& kv: p) {
            auto x = kv.second;
            y += SGmultiply(kv.first, c[i], x);
        }
        i++;
    }
}

/// display effects of in- and out-of-place addition
template<class T>
void testAdd(T& a, T& b) {
    cout << a << " + " << b << " = ";
    a += b;
    cout << a << ";\n";
    cout << a << " + " << b << " = " << a + b << ".\n\n";
}

/// display effects of in- and out-of-place multiplication
template<class T>
void testMul(T& a, T& b) {
    cout << a << " * " << b << " = ";
    a *= b;
    cout << a << ";\n";
    cout << a << " * " << b << " = " << a * b << ".\n\n";
}


int main(int, char**) {
    CodeVersion::display_code_version();

    /////////////
    // base types

    // arithmetic ring
    ArithmeticRing_t<int> SGA_a(3);
    ArithmeticRing_t<int> SGA_b(5);
    testAdd(SGA_a, SGA_b);
    testMul(SGA_a, SGA_b);

    // array addition Semigroup
    SGArray_t<3> SGa3_a({1,2,3});
    SGArray_t<3> SGa3_b({4,5,6});
    testAdd(SGa3_a, SGa3_b);

    // map-type Semigroup
    SGMap_t<> SGm_a({{1,2},{3,4}});
    SGMap_t<> SGm_b({{5,6},{7,8}});
    testAdd(SGm_a, SGm_b);

    ////////////////////////
    // construct polynomials

    /// univariate unsigned integer exponents polynomial
    typedef AbstractPolynomial<ArithmeticRing_t<precision_t>, ArithmeticRing_t<int>> P1_t;
    P1_t P1_a({{1,2}, {3,4}});
    P1_t P1_b({{5,6}, {7,8}});
    testAdd(P1_a, P1_b);
    testMul(P1_a, P1_b);


    /// 3-variable polynomial
    typedef Polynomial_t<3,precision_t> P3_t;
    SGArray_t<3> x({1,0,0}), y{{0,1,0}};
    P3_t P3_a({{x, 4.}, {y, 6.}});
    P3_t P3_b({ { {{0,0,1}}, 2. }, { {{1,0,1}}, 5.}});
    testAdd(P3_a, P3_b);
    testMul(P3_a, P3_b);

    /// unlimited-variable polynomial
    typedef PolynomialM_t<precision_t> PM_t;
    PM_t PM_a({ { {{ {0,3} }}, 4.}, { {{ {5,1} }}, 6.} });
    PM_t PM_b({ { {{ {37,2} }}, 7.}, { {{ {101,0} }}, 8.} }); // note x^0 kept if explicitly constructed...
    testAdd(PM_a, PM_b);
    testMul(PM_a, PM_b);

    ///////////////////////
    // Evaluate polynomials

    auto p = P3_a;
    //PM_t p({ { {{ {0,3} }}, 4.}, { {{ {1,2} }}, 6.} });

    PolyEval<precision_t> PE;
    int ntrials = 100;
    vector<PolyEval<precision_t>::coord_t<3>> vc(1000000/ntrials);
    for(size_t i=0; i<vc.size(); i++) {
        vc[i] = {0.5*i*1e-5, sqrt(i/1e4), i*1.e-10*i};
    }
    vector<precision_t> vp, vp2;

    cout << "evaluating " << p << "\n";

    auto t0 = steady_clock::now();

    for(int i=0; i<ntrials; i++) addSimple(p, vp2, vc);

    auto t1 = steady_clock::now();

    for(int i=0; i<ntrials; i++) {
        PE.setX(vc);
        PE.addPolynomial(p, vp);
    }

    auto t2 = steady_clock::now();

    printf("dt1 = %g; dt2 = %g\n",
           std::chrono::duration<double>(t1-t0).count(),
           std::chrono::duration<double>(t2-t1).count());

    double dmax =  0;
    for(size_t i=0; i<vp.size(); i++) dmax = std::max(dmax, fabs((vp[i]-vp2[i])/vp[i]));
    printf("dmax %g\n", dmax);

    //////////////////////////////////////////////
    // all third-order 3-variable terms polynomial

    NGrid<3, P3_t::exp_t> NG0({4,4,4});
    P3_t P3_o3;
    for(auto& a: NG0) if(a[0]+a[1]+a[2] < 4) P3_o3[a] = 3.14;
    cout << P3_o3 << "\n";

#if false
    // calculus test
    auto pi2 = p.integral(2);
    auto dpi2 = pi2.derivative(2);
    std::cout << pi2 << "\n" << dpi2 << "\n";
    assert(p == dpi2);
#endif

    ///////////////////////////
    // generate evaluation grid

    BBox<3,precision_t> BB;
    BB.expand({precision_t(-1),precision_t(-1),precision_t(-1)});
    BB.expand({precision_t(1),precision_t(1),precision_t(1)});
    NGrid<3> NG({5,5,5});
    vc.clear();
    for(auto c: NG) {
        auto x = NG.centerpos(c,BB);
        //printf("%i,%i,%i -> %g\t%g\t%g\n", c[0], c[1], c[2], double(x[0]), double(x[1]), double(x[2]));
        vc.push_back(x);
    }

    //////////////////////////////////
    // fit polynomial over grid points

    PolyFit<P3_t> PF(P3_o3);
    std::cout << PF.getPoly() << "\n";

    PF.setX(vc);
    vector<precision_t> yy;
    PE.setX(vc);
    PE.evalPolynomial(PF.getPoly(), yy);
    auto& PP = PF.solve(yy);
    std::cout << PP << "\n" << PF.ssresid() << "\n";

    return EXIT_SUCCESS;
}
