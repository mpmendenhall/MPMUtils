/// \file testZCircuit.cc Modeling network of linear impedance devices

#include "ConfigFactory.hh"
#include "FFTW_Convolver.hh"
#include "ZCircuit.hh"
#include "Rational.hh"
#include <TGraph.h>
#include <TAxis.h>
#include <TPad.h>

struct FiltInfo {
    LRCFilterBase<>& F;
    Zcalc<>& Z1;
    Zcalc<>& Z2;
};

typedef std::complex<double> cplx_t;

/// Calculate filter response
void ZFilter(vector<double>& v, FiltInfo F) {
    // convert to complex-valued k-space
    const auto N = v.size();
    auto& FFTer = IFFTWorkspace<R2CPlan<double>>::get_iffter(N);
    FFTer.v_x = v;
    FFTer.execute();

    // apply filter response
    int k = 0;
    for(auto& c: FFTer.v_k) {
        auto w = k*2*M_PI/N;
        c *= F.F.setZ(F.Z1(w), F.Z2(w));
        k++;
    }

    // return to real space
    FFTer.etucexe();
    v.assign(FFTer.v_x.begin(), FFTer.v_x.end());
}

typedef std::complex<Rational> rcplx_t;

template<>
bool mag_lt<rcplx_t>(const rcplx_t& a, const rcplx_t& b) { return std::norm(a) < std::norm(b); }


REGISTER_EXECLET(testZCircuit) {
    rcplx_t R100 = {100, 0};

    ZCircuit<1, rcplx_t> C;
    C.addLink(0, 2, R100);
    C.addLink(0, 1, R100);

    C.Vnodes.push_back({0});    // ground
    C.Vnodes.push_back({10,0}); // source voltage

    C.solve();
    std::cout << C;
    std::cout << C.M << C.RHS << C.Mi << C.V << "\n\n\n";

    //------------------------------------

    typedef LRCFilter<3> Filter_t;
    Filter_t LF(true);
    LF.addLink(0, Filter_t::iGnd, 50);                      // input termination
    LF.addLink(Filter_t::NNodes - 1, Filter_t::iGnd, 50);   // output termination

    // check purely resistive divider
    R_Zcalc<> ZR(1);
    LF.setZ(ZR(0), ZR(0));
    std::cout << LF << LF.M << LF.Mi << LF.RHS << LF.V << "****************\n\n";

    C_Zcalc<> ZC(.5, 2);
    L_Zcalc<> ZL(24, 2);
    //C_Zcalc<> ZC(1, .3);
    //L_Zcalc<> ZL(12, .3);
    FiltInfo FI{LF, ZL, ZC};

    size_t N = 512;
    vector<double> d(N); d[N/4] = 1;

    ZFilter(d, FI);

    auto dx = d;
    N = d.size();
    while(N--) dx[N] = N;

    TGraph gImpulse(d.size(), dx.data(), d.data());
    gImpulse.SetTitle("filter impulse response");

    gImpulse.SetLineColor(2);
    gImpulse.Draw("AL");
    gImpulse.GetXaxis()->SetTitle("time");
    gImpulse.GetYaxis()->SetTitle("voltage");

    gPad->Print("FilterImpulse.pdf");
}
