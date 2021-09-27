/// \file testFitters.cc Test of fit routines

#include "ConfigFactory.hh"
#include "PolyFit.hh"
#include "LinMinConstrained.hh"
#include "NGrid.hh"
#include "BBox.hh"
#include <stdlib.h>
using std::cout;

typedef double precision_t;

REGISTER_EXECLET(testFitters) {
    //////////////////////////////////////////////
    // all third-order 3-variable terms polynomial

    /// 3-variable polynomial
    typedef Polynomial_t<3,precision_t> P3_t;
    NGrid<3, unsigned int> NG0({4,4,4});
    P3_t P3_o3;
    for(auto& a: NG0) if(a[0]+a[1]+a[2] < 4) P3_o3[a] = 3.14;
    //cout << P3_o3 << "\n";

    ///////////////////////////
    // generate evaluation grid

    BBox<3,precision_t> BB;
    BB.expand({precision_t(-1),precision_t(-1),precision_t(-1)});
    BB.expand({precision_t(1),precision_t(1),precision_t(1)});
    NGrid<3> NG({5,5,5});

    vector<PolyEval<precision_t>::coord_t<3>> vc;
    for(auto c: NG) vc.push_back(NG.centerpos(c,BB));

    //////////////////////////////////
    // fit polynomial over grid points

    LinMin LM(P3_o3.size());
    PolyFit<P3_t> PF(P3_o3);
    std::cout << PF.P << "\n";

    PF.setX(vc);
    PF.configure(LM);

    vector<precision_t> yy;
    PolyEval<precision_t> PE;
    PE.setX(vc);
    PE.evalPolynomial(PF.P, yy);
    LM.solve(yy);
    const auto& PP = PF.load(LM);
    std::cout << PP << "\n" << LM.ssresid() << "\n";

    ///////////////////////
    // constrained fit test

    typedef Polynomial_t<1,precision_t> P1_t;
    P1_t px(1,0);
    px = px.pow(0) + px.pow(1) + px.pow(2);
    cout << px << "\n";

    vector<PolyEval<precision_t>::coord_t<1>> vx;
    vector<precision_t> vy;
    for(int i=0; i<20; i++) {
        vx.push_back({i*1.});
        vy.push_back(0.5 + 0.3*i + 0.6*i*i);
    }

    PolyFit<P1_t> PF1(px);
    PF1.setX(vx);

    LinMinConstrained LMC(px.size());
    PF1.configure(LMC);
    LMC.setNConstraints(1);
    LMC.setG(0,0,1);
    LMC.setG(0,1,1);
    LMC.setk(0,0.9);
    LMC.solve(vy);
    PF1.load(LMC);
    cout << PF1.P << "\n";
}
