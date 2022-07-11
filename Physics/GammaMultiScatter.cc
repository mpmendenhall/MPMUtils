/// \file GammaMultiScatter.cc

#include "GammaMultiScatter.hh"
#include <gsl/gsl_integration.h>
#include <TAxis.h>

GammaScatterSteps::GammaScatterSteps(double _E0, double _eDens, int _npts):
E0(_E0), eDens(_eDens), npts(_npts), steps(1) {

    // calculate cross-section, escape fractions over relevant energy range
    double s_tot = 0;
    for(int i=0; i<npts; ++i) {
        auto E1 = i*E0/(npts - 1);
        s_tot = KN_total_xs(E1/m_e);
        gCx.SetPoint(i, E1, s_tot);
        Escape_0 = exp(-eDens * N_A * s_tot * 1e-24);
        gEsc.SetPoint(i, E1, Escape_0);
    }
    gCx.SetBit(TGraph::kIsSortedX);
    gEsc.SetBit(TGraph::kIsSortedX);
    Scatter_0 = 1. - Escape_0;

    gEsc.SetMinimum(0);
    gEsc.GetYaxis()->SetTitle("unscattered fraction");
    gEsc.GetXaxis()->SetTitle("gamma energy [MeV]");

    gCx.SetMinimum(0);
    gCx.GetXaxis()->SetTitle("gamma energy [MeV]");
    gCx.GetYaxis()->SetTitle("total scattering cross-section [barn]");

    // first scattering step from delta-function input
    const double Em = E0/m_e;
    double fmin = gamma_escatter_fmin(Em);
    auto& gI = steps.at(0).Incident;
    steps.at(0).Emin = fmin * E0;
    for(int i = 0; i < npts; ++i) {
        auto f = exp((1-i/double(npts - 1))*log(fmin));
        //auto f = fmin + i*(1-fmin)/(npts-1);
        double s = KN_ds_df(Em, f);
        gI.SetPoint(i, f*E0, Scatter_0 * s/(s_tot * E0));
    }
    gI.GetXaxis()->SetTitle("gamma energy [MeV]");
    gI.GetYaxis()->SetTitle("incident spectrum [/gamma/MeV]");
    gI.SetBit(TGraph::kIsSortedX);
    splitIncident();
    Escape = steps.at(0).Escape;
}

TGraph GammaScatterSteps::Egamma_to_Ee(const TGraph& g) const {
    int n = g.GetN();
    TGraph ge(n);
    for(int i = 0; i < n; ++i) ge.SetPoint(n-i-1, E0 - g.GetX()[i], g.GetY()[i]);
    ge.SetBit(TGraph::kIsSortedX);
    ge.GetYaxis()->SetTitle(g.GetYaxis()->GetTitle());
    ge.GetXaxis()->SetTitle("scattered electron energy [MeV]");
    return ge;
}

double _eval_tgraph(double x, void* p) { return static_cast<tgraph_integrator*>(p)->g.Eval(x); }

double tgraph_integrator::integrate(double x0, double x1, double epsab, double epsrel) {
    gsl_function f;
    f.params = this;
    f.function = &_eval_tgraph;

    if(nadaptive) {
        auto iw = gsl_integration_workspace_alloc(nadaptive);
        gsl_integration_qags(&f, x0, x1, epsab, epsrel, nadaptive, iw, &res, &abserr);
        gsl_integration_workspace_free(iw);
    } else {
        gsl_integration_qng(&f, x0, x1, epsab, epsrel, &res, &abserr, &neval);
    }

    return res;
}

struct scatter_integ_params {
    const TSpline3& gS;
    const TGraph& gEsc;
    const TGraph& gCx;
    double E;
};

double scatter_integral(double x, void* p) {
    auto& P = *static_cast<scatter_integ_params*>(p);
    auto y = P.gS.Eval(x) * KN_ds_df(x/m_e, P.E/x);
    return y;
}

void GammaScatterSteps::scatter_step() {
    scatter_integ_params P{steps.back().sScatter, gEsc, gCx, 0.};
    gsl_function f;
    f.params = &P;
    f.function = &scatter_integral;

    TGraph gI;
    auto EminPrev = steps.back().Emin;
    double Emin = gamma_escatter_fmin(EminPrev/m_e) * EminPrev;
    auto Emin0 = steps.at(0).Emin;

    size_t nadaptive = 100;
    auto iw = nadaptive? gsl_integration_workspace_alloc(nadaptive) : nullptr;

    for(int i=0; i<npts; ++i) {
        double l = i/double(npts - 1);
        P.E = exp((1-l)*log(Emin) + l*log(E0));

        double Emax = std::min(gamma_escatter_Emax_per_m_e(P.E/m_e)*m_e, E0);
        size_t neval = 0;
        double res = 0, abserr = 0;
        double EI0 = std::max(P.E, EminPrev);


        // split integration around Emin0 cusp in second step
        double s = 0;
        if(steps.size() == 2 && EI0 < Emin0 && Emin0 < Emax) {
            if(nadaptive) gsl_integration_qags(&f, EI0, Emin0, 1e-4, 1e-3, nadaptive, iw, &res, &abserr);
            else gsl_integration_qng(&f, EI0, Emin0,  1e-4, 1e-3, &res, &abserr, &neval);
            s += res;
            if(nadaptive) gsl_integration_qags(&f, Emin0, Emax, 1e-4, 1e-3, nadaptive, iw, &res, &abserr);
            else gsl_integration_qng(&f, Emin0, Emax, 1e-4, 1e-3, &res, &abserr, &neval);
            s += res;
        } else {
            if(nadaptive) gsl_integration_qags(&f, EI0, Emax, 1e-4, 1e-3, nadaptive, iw, &res, &abserr);
            else gsl_integration_qng(&f, P.E, Emax,   1e-4, 1e-3, &res, &abserr, &neval);
            s = res;
        }
        gI.SetPoint(i, P.E, s);
    }

    gsl_integration_workspace_free(iw);

    steps.push_back({gI, {}, {}, {}, {}, Emin});
    splitIncident();
    sumEscaped();
}

void GammaScatterSteps::splitIncident() {
    auto& S = steps.back();
    const auto& gI = S.Incident;
    auto& gE = S.Escape = gI;
    auto& gS = S.Scatter = gI;
    for(int i=0; i < gI.GetN(); ++i) {
        auto f = gEsc.Eval(gI.GetX()[i]);
        gE.GetY()[i] *= f;
        gS.GetY()[i] *= 1 - f;
    }

    tgraph_integrator tgi(gS, 50);
    auto Emin0 = steps.at(0).Emin;
    S.nScatter = tgi.integrate(Emin0, E0);
    if(S.Emin < Emin0) S.nScatter += tgi.integrate(S.Emin, Emin0);

    // "normalize scattering distribution to unit area dE"
    for(int i=0; i < gS.GetN(); ++i) {
        auto x = gS.GetX()[i];
        gS.GetY()[i] /=  x * gCx.Eval(x);
    }

    S.sScatter = TSpline3("sScatter", &gS);

    gE.GetYaxis()->SetTitle("gamma escape probability [/gamma/MeV]");
    gS.GetYaxis()->SetTitle("scattering [/gamma/MeV/#sigma_{tot}(E)]");
}

void GammaScatterSteps::sumEscaped() {
    auto& gE = steps.back().Escape;
    double x0 = Escape.GetX()[0];
    TGraph g = gE;
    int i = 0;
    for(; i < gE.GetN(); ++i) if(gE.GetX()[i] >= x0) break;
    g.Set(i);
    g.SetPoint(i++, x0, gE.Eval(x0));

    for(int j = 0; j < Escape.GetN(); ++j) {
        auto x = Escape.GetX()[j];
        g.SetPoint(i++, x, Escape.GetY()[j] + gE.Eval(x));
    }
    Escape = g;
}
