/// @file GammaMultiScatter.cc

#include "GammaMultiScatter.hh"
#include "SmearingIntegral.hh"
#include "GraphUtils.hh"
#include <TAxis.h>

struct scatter_integ_params {
    const TSpline3& gS;
    double E;
};

// outgoing gammas at energy P.E from incident gamma energy x
double scatter_integral(double x, void* p) {
    auto& P = *static_cast<scatter_integ_params*>(p);
    return P.gS.Eval(x) * KN_ds_df(x/m_e, P.E/x);
}

// compton scatters at electron energy P.E from incident gamma energy x
double compton_integral(double x, void* p) {
    auto& P = *static_cast<scatter_integ_params*>(p);
    return P.gS.Eval(x) * KN_ds_df(x/m_e, (x - P.E)/x);
}


////////////////////////////////
////////////////////////////////
////////////////////////////////


GammaScatterSteps::GammaScatterSteps(double _E0, double _eDens, double _Z, int _npts):
E0(_E0), eDens(_eDens), Z(_Z), npts(_npts), scatterIntegrator(100) {

    scatterIntegrator.f.function = &scatter_integral;
    eScatterIntegrator.f.function = &compton_integral;
    if(eDens > 0) setDens(eDens);

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
}

void GammaScatterSteps::setDens(double _eDens, size_t nsteps) {
    eDens = _eDens;
    calcIxns();
    while(nsteps--) scatter_step();
}

GammaScatterSteps::s_Interactions GammaScatterSteps::interactionsAt(double E) const {
    auto x = E/m_e;
    s_Interactions I;
    I.s_Compt = KN_total_xs(x);
    I.s_PE = photoelectric_cx_1965(x, Z);
    I.p_Ixn = 1 - exp(-eDens * N_A * (I.s_Compt + I.s_PE) * 1e-24);
    I.f_Compt = I.s_Compt / (I.s_Compt + I.s_PE);
    return I;
}

void GammaScatterSteps::calcIxns() {

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

    sPE = TSpline3("sPE", &gPE);

    // first scattering step from delta-function input
    const double Em = E0/m_e;
    double fmin = gamma_escatter_fmin(Em);

    steps.resize(1);
    auto& S = steps.at(0) = {{}, fmin * E0, E0, E0};

    auto& gI = S.Incident;
    for(int i = 0; i < npts; ++i) {
        auto f = exp((1-i/double(npts - 1))*log(fmin)); // log spacing
        double s = KN_ds_df(Em, f);
        gI.SetPoint(i, f*E0, Scatter_0 * s/(I.s_Compt * E0));
    }
    gI.SetBit(TGraph::kIsSortedX);
    gI.GetXaxis()->SetTitle("gamma energy [MeV]");
    gI.GetYaxis()->SetTitle("incident spectrum [/gamma/MeV]");

    splitIncident(S);
    FullCapt += S.fullCapt;
    Escape = S.EscapeSum = S.Escape;
}

void GammaScatterSteps::scatter_step() {
    if(!steps.size()) throw std::logic_error("First step uninitialized");

    scatter_integ_params P{steps.back()._Scatter, 0.};
    scatterIntegrator.setParams(&P);

    TGraph gI;
    gI.GetXaxis()->SetTitle("gamma energy [MeV]");
    gI.GetYaxis()->SetTitle("incident spectrum [/gamma/MeV]");
    auto EminPrev = steps.back().Emin;
    double Emin = gamma_scatter_Emin(EminPrev);

    for(int i=0; i<npts; ++i) {
        double l = i/double(npts - 1);
        P.E = exp((1-l)*log(Emin) + l*log(E0));

        double Emax = std::min(gamma_escatter_Emax_per_m_e(P.E/m_e)*m_e, E0);
        double EI0 = std::max(P.E, EminPrev);

        // split integration around Ec cusp in second step
        auto Ec = steps.at(0).Emax;
        double s = 0;
        if(steps.size() == 2 && EI0 < Ec && Ec < Emax) {
            s  = scatterIntegrator.integrate(EI0, Ec);
            s += scatterIntegrator.integrate(Ec, Emax);
        } else {
            s  = scatterIntegrator.integrate(EI0, Emax);
        }
        gI.SetPoint(i, P.E, s);
    }

    scatterIntegrator.setParams(nullptr); // just to be safe

    steps.emplace_back(gI, Emin, EminPrev, E0);
    splitIncident(steps.back());
    FullCapt += steps.back().fullCapt;
    sumEscaped();
}

void GammaScatterSteps::splitIncident(s_ScatterStep& S) const {
    const auto& gI = S.Incident;
    auto& gE = S.Escape = gI;
    gE.GetYaxis()->SetTitle("gamma escape probability [/gamma/MeV]");
    auto gS = gI;   // Compton scattering portion of incident
    auto gP = gI;   // Photoelectric portion of incident
    for(int i=0; i < gI.GetN(); ++i) {
        auto I = interactionsAt(gI.GetX()[i]);
        gE.GetY()[i] *= 1 - I.p_Ixn;
        gS.GetY()[i] *= I.p_Ixn * I.f_Compt;
        gP.GetY()[i] *= I.p_Ixn * (1 - I.f_Compt);
    }
    TSpline3 sS("_Scatter_tmp", &gS);
    TSpline3 sP("sPhotoelectric", &gP);

    // total scattering and photoelectric capture, splitting integration at first Compton edge
    tspline_integrator tgi(sS, 50); // Compton + Photoelectric integration
    tspline_integrator tge(sP, 50); // Photoelectric
    S.nScatter = S.fullCapt = 0;
    auto Ec = steps.at(0).Emax;
    if(S.Emin < Ec && Ec < S.Emax) {
        S.nScatter += tgi.integrate(S.Emin, Ec);
        S.fullCapt += tge.integrate(S.Emin, Ec);
        S.nScatter += tgi.integrate(Ec, S.Emax);
        S.fullCapt += tge.integrate(Ec, S.Emax);
    } else {
        S.nScatter += tgi.integrate(S.Emin, S.Emax);
        S.fullCapt += tge.integrate(S.Emin, S.Emax);
    }

    // normalize scattering distribution to unit area dE
    for(int i = 0; i < gS.GetN(); ++i) {
        auto x = gS.GetX()[i];
        gS.GetY()[i] /=  x * KN_total_xs(x/m_e);
    }
    S._Scatter = TSpline3("_Scatter", &gS);
}

void GammaScatterSteps::sumEscaped() {
    if(!steps.size()) return;

    auto& gE = steps.back().Escape;

    // new values up to previous lowest energy x0
    double x0 = Escape.GetX()[0];
    TGraph g = gE;
    int i = 0;
    for(; i < gE.GetN(); ++i) if(gE.GetX()[i] >= x0) break;
    g.Set(i);
    g.SetPoint(i++, x0, gE.Eval(x0));
    g.SetBit(TGraph::kIsSortedX);
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
    g.SetBit(TGraph::kIsSortedX);
    Escape = g;
}


////////////////////////////////
////////////////////////////////
////////////////////////////////


TGraph GammaScatterSteps::Egamma_to_Ee(const TGraph& g) const {
    int n = g.GetN();
    TGraph ge(n);
    for(int i = 0; i < n; ++i) ge.SetPoint(n-i-1, E0 - g.GetX()[i], g.GetY()[i]);
    ge.SetBit(TGraph::kIsSortedX);
    ge.GetYaxis()->SetTitle(g.GetYaxis()->GetTitle());
    ge.GetXaxis()->SetTitle("scattered electron energy [MeV]");
    return ge;
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
    for(const auto& S: steps) csegs.push_back(Egamma_to_Ee(S.EscapeSum));

    double Erange = E0 + 4 * sqrt(E0/PE_per_MeV);
    for(int i = 0; i < npts; ++i) {
        double x = i * Erange/(npts-1);

        // full capture peak
        double dx = x - E0;
        double s2 = E0 / PE_per_MeV;
        double y = exp(-dx*dx/(2*s2))/sqrt(2*M_PI*s2) * (steps.back().nScatter + FullCapt);

        // separately integrate each segment to avoid singularities
        for(const auto& gs: csegs) y += GSI.apply(gs, x);

        gSmear.SetPoint(i, x, y);
    }
    return gSmear;
}



//////////////////////////////////////////
//////////////////////////////////////////
//////////////////////////////////////////


double GammaScatterSteps::comptonsFrom(const s_eScatterStep& S, double E) {
    // incident gamma range capable of producing electrons of this energy
    double Emin = std::max(gamma_Emin_for_Compton(E), S.Emin);
    if(!(Emin < S.Emax)) return 0;

    scatter_integ_params P{S._Scatter, E};
    eScatterIntegrator.setParams(&P);
    auto i = eScatterIntegrator.integrate(Emin, S.Emax);
    eScatterIntegrator.setParams(nullptr);
    return i;
}

GammaScatterSteps::s_eScatterStep GammaScatterSteps::fromEscaping(const s_ScatterStep& S) {
    s_eScatterStep Se(S.EscapeSum, S.Emin, S.EminPrev);
    single_scatter_deposition(Se);
    return Se;
}

void GammaScatterSteps::single_scatter_deposition(s_eScatterStep& S) {
    const auto& gI = S.Incident;
    auto& gP = S.PhotoElec = gI;
    auto gS = S.Incident;       // normalized compton scattering distribution
    for(int i=0; i < gI.GetN(); ++i) {
        auto E = gI.GetX()[i];
        auto I = interactionsAt(E);
        gS.GetY()[i] *= I.p_Ixn * I.f_Compt / KN_total_xs(E/m_e);
        gP.GetY()[i] *= I.p_Ixn * (1 - I.f_Compt);
    }
    gP.GetXaxis()->SetTitle("photoelectric electron energy [MeV]");
    gP.GetYaxis()->SetTitle("photoelectric interactions [/MeV]");

    S._Scatter = TSpline3("_Scatter", &gS);
}

void GammaScatterSteps::calcRescatter(const GammaScatterSteps& GSS) {
    bSteps.clear();
    for(const auto& S: GSS.steps) bSteps.push_back(fromEscaping(S));
    bComptons = {};
    bComptons.GetXaxis()->SetTitle("Compton electron energy [MeV]");
    bComptons.GetYaxis()->SetTitle("Compton scatters [/MeV]");
    if(!bSteps.size()) return;

    auto Ec = bSteps[0].Ec;
    auto Ec2 = compton_edge_e_for_gamma(bSteps[0].Emin);
    int j = 0;
    for(int i=0; i < npts/2 - 1; ++i) {
        double E = i * Ec2/(npts/2 - 1);
        double y = 0;
        for(const auto& S: bSteps) if(E < S.Ec) y += comptonsFrom(S,E);
        bComptons.SetPoint(j++, E, y);
    }
    for(int i=0; i < npts/2-1; ++i) {
        double l = double(i)/(npts/2 - 1);
        double E = exp((1-l)*log(Ec2) + l*log(Ec)); // log spacing
        bComptons.SetPoint(j++, E, comptonsFrom(bSteps.at(0), E));
    }
    bComptons.SetPoint(j++, Ec, 0);
    bComptons.SetBit(TGraph::kIsSortedX);
}
