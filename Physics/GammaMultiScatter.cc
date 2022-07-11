/// \file GammaMultiScatter.cc

#include "GammaMultiScatter.hh"
#include "SmearingIntegral.hh"
#include <TAxis.h>

GammaScatterSteps::s_Interactions GammaScatterSteps::interactionsAt(double E) {
        auto x = E/m_e;
        s_Interactions I;
        I.s_Compt = KN_total_xs(x);
        I.s_PE = photoelectric_cx_1965(x, Z);
        I.p_Ixn = 1 - exp(-eDens * N_A * (I.s_Compt + I.s_PE) * 1e-24);
        I.f_Compt = I.s_Compt / (I.s_Compt + I.s_PE);
        return I;
}

GammaScatterSteps::GammaScatterSteps(double _E0, double _eDens, double _Z, int _npts):
E0(_E0), eDens(_eDens), Z(_Z), npts(_npts), steps(1) {

    // calculate cross-section, escape fractions over relevant energy range
    s_Interactions I;
    for(int i=0; i<npts; ++i) {
        double l = double(i)/(npts - 1);
        auto E1 = l*l*E0;
        I = interactionsAt(E1);
        if(E1) gPE.SetPoint(i-1, E1, I.s_PE);
        gCx.SetPoint(i, E1, I.s_Compt);
        gInteract.SetPoint(i, E1, I.p_Ixn);
    }
    gCx.SetBit(TGraph::kIsSortedX);
    gInteract.SetBit(TGraph::kIsSortedX);
    // distribute initial events between Photoelectric, Compton
    FullCapt = I.p_Ixn * (1. - I.f_Compt);
    Scatter_0 = I.p_Ixn * I.f_Compt;
    Escape_0 = 1 - I.p_Ixn;

    gInteract.SetMinimum(0);
    gInteract.SetMaximum(1);
    gInteract.GetYaxis()->SetTitle("interaction probability");
    gInteract.GetXaxis()->SetTitle("gamma energy [MeV]");

    gCx.SetMinimum(0);
    gCx.GetXaxis()->SetTitle("gamma energy [MeV]");
    gCx.GetYaxis()->SetTitle("total Compton scattering cross-section [barn]");

    gPE.SetMinimum(0);
    gPE.GetXaxis()->SetTitle("gamma energy [MeV]");
    gPE.GetYaxis()->SetTitle("Photoelectric cross-section [barn]");
    sPE = TSpline3("sPE", &gPE);

    // first scattering step from delta-function input
    const double Em = E0/m_e;
    double fmin = gamma_escatter_fmin(Em);
    auto& gI = steps.at(0).Incident;
    steps.at(0).Emin = fmin * E0;
    for(int i = 0; i < npts; ++i) {
        auto f = exp((1-i/double(npts - 1))*log(fmin)); // log spacing
        double s = KN_ds_df(Em, f);
        gI.SetPoint(i, f*E0, Scatter_0 * s/(I.s_Compt * E0));
    }
    gI.GetXaxis()->SetTitle("gamma energy [MeV]");
    gI.GetYaxis()->SetTitle("incident spectrum [/gamma/MeV]");
    gI.SetBit(TGraph::kIsSortedX);
    splitIncident();
    Escape = steps.at(0).EscapeSum = steps.at(0).Escape;
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

struct scatter_integ_params {
    const TSpline3& gS;
    double E;
};

double scatter_integral(double x, void* p) {
    auto& P = *static_cast<scatter_integ_params*>(p);
    auto y = P.gS.Eval(x) * KN_ds_df(x/m_e, P.E/x);
    return y;
}

void GammaScatterSteps::scatter_step() {
    scatter_integ_params P{steps.back().sScatter, 0.};
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

    steps.emplace_back(gI, Emin);
    splitIncident();
    sumEscaped();
}

void GammaScatterSteps::splitIncident() {
    auto& S = steps.back();
    const auto& gI = S.Incident;
    auto& gE = S.Escape = gI;
    auto& gS = S.Scatter = gI;
    TGraph gP = gI;
    for(int i=0; i < gI.GetN(); ++i) {
        auto I = interactionsAt(gI.GetX()[i]);
        gE.GetY()[i] *= 1 - I.p_Ixn;
        gS.GetY()[i] *= I.p_Ixn * I.f_Compt;
        gP.GetY()[i] *= I.p_Ixn * (1 - I.f_Compt);
    }

    // split integration at first Compton edge
    tgraph_integrator tgi(gS, 50);
    tgraph_integrator tge(gP, 50);
    auto Emin0 = steps.at(0).Emin;
    S.nScatter = tgi.integrate(Emin0, E0);
    FullCapt += tge.integrate(Emin0, E0);
    if(S.Emin < Emin0) {
        S.nScatter += tgi.integrate(S.Emin, Emin0);
        FullCapt += tge.integrate(S.Emin, Emin0);
    }

    // "normalize scattering distribution to unit area dE"
    for(int i=0; i < gS.GetN(); ++i) {
        auto x = gS.GetX()[i];
        gS.GetY()[i] /=  x * KN_total_xs(x/m_e);
    }

    S.sScatter = TSpline3("sScatter", &gS);

    gE.GetYaxis()->SetTitle("gamma escape probability [/gamma/MeV]");
    gS.GetYaxis()->SetTitle("scattering [/gamma/MeV/#sigma_{tot}(E)]");
}

void GammaScatterSteps::sumEscaped() {
    auto& gE = steps.back().Escape;

    // new values up to previous lowest energy x0
    double x0 = Escape.GetX()[0];
    TGraph g = gE;
    int i = 0;
    for(; i < gE.GetN(); ++i) if(gE.GetX()[i] >= x0) break;
    g.Set(i);
    g.SetPoint(i++, x0, gE.Eval(x0));
    steps.back().EscapeSum = g;

    // sum into previous sub-segments
    for(auto& S: steps) {
        if(&S == &steps.back()) break;
        auto& gSeg = S.EscapeSum;
        for(int j = 0; j < gSeg.GetN(); ++j)
            gSeg.GetY()[j] += gE.Eval(gSeg.GetX()[j]);
    }

    // sum in previous Escape values
    for(int j = 0; j < Escape.GetN(); ++j) {
        auto x = Escape.GetX()[j];
        g.SetPoint(i++, x, Escape.GetY()[j] + gE.Eval(x));
    }
    Escape = g;
}

TGraph GammaScatterSteps::eSpectrum(double PE_per_MeV) const {
    if(!PE_per_MeV) {
        auto gC = Egamma_to_Ee(Escape);
        gC.GetYaxis()->SetTitle("Electron scattering [/gamma/MeV]");
        // represent residual full-capture peak
        auto n = gC.GetN();
        auto E1 = steps.back().Emin;
        auto h = (steps.back().nScatter + FullCapt)/E1;
        gC.SetPoint(n++, E0 - E1, h);
        gC.SetPoint(n++, E0, h);
        gC.SetPoint(n++, E0, 0);
        return gC;
    }

    TGraph gSmear;
    gaussian_smearing_integral GSI(PE_per_MeV);

    vector<TGraph> csegs;
    for(auto& S: steps) csegs.push_back(Egamma_to_Ee(S.EscapeSum));

    double Erange = E0 + 4 * sqrt(E0/PE_per_MeV);
    for(int i = 0; i < npts; ++i) {
        double x = i * Erange/(npts-1);

        // full capture peak
        double dx = x - E0;
        double s2 = E0 / PE_per_MeV;
        double y = exp(-dx*dx/(2*s2))/sqrt(2*M_PI*s2) * (steps.back().nScatter + FullCapt);

        // separately integrate each segment to avoid singularities
        for(auto& gs: csegs) y += GSI.apply(gs, x);

        gSmear.SetPoint(i, x, y);
    }
    return gSmear;
}
