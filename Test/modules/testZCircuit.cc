/// \file testZCircuit.cc Modeling network of linear impedance devices

#include "ConfigFactory.hh"
#include "GlobalArgs.hh"
#include "FFTW_Convolver.hh"
#include "FilterCircuits.hh"
#include "PoleFinder.hh"
#include "Rational.hh"
#include <TGraph.h>
#include <TAxis.h>
#include <TPad.h>

/// Plot filter circuit response
template<class CE_t = CircuitEvaluator<>>
void plotCircuit(CE_t& CE, const size_t N, double tsamp) {
    tsamp *= 1e-9; // as nanoseconds, in s

    // convert to complex-valued k-space
    auto& FFTer = IFFTWorkspace<R2CPlan<double>>::get_iffter(N);
    for(auto& x: FFTer.v_x) x = 0;
    FFTer.v_x[N/8] = 1;
    FFTer.execute();

    // apply filter response
    int k = 0;
    for(auto& c: FFTer.v_k) {
        auto w = 2 * M_PI * k / (N * tsamp);
        c *= CE({{},w});
        ++k;
    }
    printf("\nFilter integral: %g\n\n", sqrt(std::norm(FFTer.v_k[0])));

    // return to real space
    FFTer.etucexe();

    vector<double> dx(N);
    auto N0 = N;
    while(N0--) dx[N0] = (N0 - double(N/8)) * 1e9 * tsamp;

    TGraph gImpulse(N, dx.data(), FFTer.v_x.data());
    gImpulse.SetTitle("filter impulse response");

    gImpulse.SetLineColor(2);
    gImpulse.Draw("AL");
    gImpulse.GetXaxis()->SetTitle("time [ns]");
    gImpulse.GetYaxis()->SetTitle("output voltage");

    gPad->Print("FilterImpulse.pdf");

    vector<double> f(2*N);
    vector<double> R(2*N);
    vector<double> delay(2*N);
    for(k = 0; k < int(2*N); ++k) {
        f[k] = k/(N * tsamp)/1e6;
        auto w = 2e6 * M_PI * f[k];
        auto u = CE({{},w});
        R[k] = 10*log10(std::norm(u));
        delay[k] = k? 1e9 * std::arg(u)/w : 0;
    }

    printf("DC attenuation %g dB\n", R[0]);

    TGraph gDelay(delay.size(), f.data(), delay.data());
    gDelay.SetTitle("filter delay");

    gDelay.SetLineColor(2);
    gDelay.Draw("AL");
    gDelay.GetXaxis()->SetRangeUser(0, 2e3);
    gDelay.GetXaxis()->SetTitle("f [MHz]");
    gDelay.GetYaxis()->SetTitle("delay [ns]");

    gPad->Print("FilterDelay.pdf");

    gPad->SetLogx(true);

    TGraph gResponse(R.size(), f.data(), R.data());
    gResponse.SetTitle("filter frequency response");

    gResponse.SetLineColor(2);
    gResponse.Draw("AL");
    gResponse.GetXaxis()->SetRangeUser(500/(N * tsamp), 2e3/tsamp);
    gResponse.GetXaxis()->SetTitle("f [MHz]");
    gResponse.GetYaxis()->SetTitle("attenuation [dB]");

    gPad->Print("FilterFreq.pdf");
}

typedef std::complex<Rational> rcplx_t;
template<>
bool mag_lt<rcplx_t>(const rcplx_t& a, const rcplx_t& b) { return std::norm(a) < std::norm(b); }

/// Stuffer for 2-identical-components ladder circuit
template<class ZCS_t = ZCircuitStuffer<>>
class BinaryLadderStuffer: public ZCS_t {
public:
    using typename ZCS_t::ZCalc_t;
    using ZCS_t::ps;

    /// Constructor
    BinaryLadderStuffer(ZCalc_t& Z1, ZCalc_t& Z2) {
        ps.emplace_back(&Z1);
        ps.emplace_back(&Z2);
    }

    /// Setup stuffing for N-node ladder filter
    void configure(size_t N) {
        ps[0].links.clear();
        ps[1].links.clear();

        for(size_t i=0; i < N-1; ++i) {
            ps[0].links.push_back(2*i);
            ps[1].links.push_back(2*i + 1);
        }
    }
};


REGISTER_EXECLET(testZCircuit) {
    rcplx_t R100 = {100, 0};

    ZCircuit<1, rcplx_t> C;
    C.Vnodes.push_back({0});    // ground
    C.Vnodes.push_back({10,0}); // source voltage
    C.addLink(0, 2, R100);
    C.addLink(0, 1, R100);

    C.solve();
    std::cout << C;
    std::cout << C.M << C.RHS << C.Mi << C.V << "\n\n\n";

    //------------------------------------

    double R_out = 50;
    optionalGlobalArg("rout", R_out, "output termination resistor [ohms]");
    double _C = 4;
    double _L = 10;
    double _CR = 0.6;
    double _LR = 0.05;
    double tgrid = 2;
    int ngrid = 256;
    optionalGlobalArg("C", _C, "filter capcitors capacitance [nF]");
    optionalGlobalArg("L", _L, "filter inductors inductance [nH]");
    optionalGlobalArg("CR", _CR, "filter capcitors series resistance");
    optionalGlobalArg("LR", _LR, "filter inductors series resistance");
    optionalGlobalArg("tgrid", tgrid, "calculation grid spacing [ns]");
    optionalGlobalArg("ngrid", ngrid, "number of calculation gridpoints");

    ZCircuit_Base<>* Circ = nullptr;
    if(wasArgGiven("onestage", "Single-stage filter")) Circ = new ZCircuit<2>();
    else Circ = new ZCircuit<3>();
    configure_Ladder(*Circ);
    if(R_out) Circ->addLink(Circ->iOut, Circ->iGnd, {R_out,0});

    C_ZCalc<> ZC(1e-9 * _C, _CR);
    L_ZCalc<> ZL(1e-9 * _L, _LR);
    BinaryLadderStuffer<> BLS(ZL, ZC);
    BLS.configure(Circ->Ncalc);
    BLS.setS({{}, 2e7});
    BLS.stuff(*Circ);
    std::cout << *Circ;

    CircuitEvaluator<> CE(BLS, *Circ);
    if(wasArgGiven("fit", "Fit circuit poles")) {
        PoleFinder<> PF;
        PF.scan_grid(CE, {-5e8, 0}, {0, 3e8});
        PF.fit(CE);
        plotCircuit(PF, ngrid, tgrid);
    }
    else plotCircuit(CE, ngrid, tgrid);
}
