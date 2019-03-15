/// \file testPolynomial.cc Test of polynomial manipulation

#include "Polynomial.hh"
#include "PolyEval.hh"
#include "PolyFit.hh"
#include "CodeVersion.hh"
#include "NGrid.hh"
#include "BBox.hh"
#include "TestOperators.hh"
#include <stdlib.h>
#include <chrono>
#include "Abstract.hh"
using std::chrono::steady_clock;
using std::cout;

//typedef float precision_t;
typedef double precision_t;
//typedef long double precision_t;
//typedef __float128 precision_t;


/// apply semigroup operator as multiply
template<typename SG, typename T, typename Data>
T& SGmultiply(const SG& o, const Data& d, T& x0) {
    auto ts = o.get();
    for(auto& kv: ts) {
        auto e = kv.second;
        while(e-- > 0) x0 *= d[kv.first];
    }
    return x0;
}

/// apply semigroup operator as add
template<typename SG, typename T, typename Data>
T& SGadd(const SG& o, const Data& d, T& x0) {
    auto ts = o.get();
    for(auto& kv: ts) {
        auto e = kv.second;
        while(e-- > 0) x0 += d(kv.first);
    }
    return x0;
}

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

int main(int, char**) {
    CodeVersion::display_code_version();

    // array addition Semigroup
    SGArray_t<3> SGa3_a({1,2,3});
    SGArray_t<3> SGa3_b({4,5,6});
    //testAdd(SGa3_a, SGa3_b);

    // map-type Semigroup
    SGMap_t<> SGm_a({{1,2},{3,4}});
    SGMap_t<> SGm_b({{5,6},{7,8}});
    //testAdd(SGm_a, SGm_b);

    // sorted-vector Semigroup
    SGVec_t<> SGv_a(1,2);
    SGVec_t<> SGv_b(3,4);
    //testAdd(SGv_a, SGv_b);

    ////////////////////////
    // construct polynomials

    /// univariate unsigned integer exponents polynomial
    typedef Pol1_t<precision_t> P1_t;
    P1_t P1_a({{1,2}, {3,4}});
    P1_t P1_b({{5,6}, {7,8}});
    testAdd(P1_a, P1_b);
    testMul(P1_a, P1_b);


    /// 3-variable polynomial
    typedef Polynomial_t<3,precision_t> P3_t;
    //SGArray_t<3> x({1,0,0}), y{{0,1,0}};
    //P3_t P3_a({{x, 4.}, {y, 6.}});
    P3_t Px(0);
    P3_t Py(1);
    auto P3_a = Px*Px*Px*Px + Py*Py*Py*Py*Py*Py;
    P3_t P3_b({ { {{0,0,1}}, 2. }, { {{1,0,1}}, 5.}});
    testAdd(P3_a, P3_b);
    testMul(P3_a, P3_b);

    /// unlimited-variable polynomial
    typedef PolynomialM_t<precision_t> PM_t;
    PM_t PM_a({ { {{ {0,3} }}, 4.}, { {{ {5,1} }}, 6.} });
    PM_t PM_b({ { {{ {37,2} }}, 7.}, { {{ {101,0} }}, 8.} }); // note x^0 kept if explicitly constructed...
    testAdd(PM_a, PM_b);
    testMul(PM_a, PM_b);

    /// sorted-vector unlimited-variable polynomial
    typedef PolynomialV_t<precision_t> PV_t;
    PV_t PV_a({ {{1,2}, 3}, {{4,5}, 6} });
    PV_t PV_b({ {{7,8}, 9} });
    testAdd(PV_a, PV_b);
    testMul(PV_a, PV_b);

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

#if false
    // calculus test
    auto pi2 = p.integral(2);
    auto dpi2 = pi2.derivative(2);
    std::cout << pi2 << "\n" << dpi2 << "\n";
    assert(p == dpi2);
#endif

    //AbstractPolynomial<Rational, SemigroupPlus<int>> Pr({{1,2}, {3,{4,5}}});
    //Pr += Rational(1,2);
    //std::cout << Pr << Pr.pow(5) << "\n\n\n";

    return EXIT_SUCCESS;
}
