/// \file GammaEdgeFitter.cc

#include "GammaEdgeFitter.hh"
#include "SmearingIntegral.hh"

GammaEdgeFitter::GammaEdgeFitter(double _E0):
TF1("GammaEdge", this, &GammaEdgeFitter::Evaluate, 0, 1, 4, "MyFunction","Evaluate"),
GammaScatterSteps(_E0, 0) {
    SetLineColor(6);
    SetParameter(1, rate);
}

void GammaEdgeFitter::UpdateCore() {
    calcIxns();

    // calculate scattering steps
    for(int i = 0; i < 100; ++i) {
        if(i) scatter_step();
        auto& S = steps.back();
        if(S.nScatter < 1e-3 * Scatter_0) break;
    }

    csegs.clear();
    for(auto& S: steps) csegs.push_back(Egamma_to_Ee(S.EscapeSum));
}

double GammaEdgeFitter::Evaluate(double* x, double* p) {
    SigPerE     = p[0];
    rate        = p[1];
    PE_per_MeV  = p[2];

    // update model for parameter changes
    bool do_up = eDens != p[3];
    eDens = p[3];
    if(do_up) UpdateCore();

    gaussian_smearing_integral GSI(PE_per_MeV);

    // full capture peak
    auto E = (*x)/SigPerE;
    double dx = E - E0;
    double s2 = E0 / PE_per_MeV;
    double y = exp(-dx*dx/(2*s2))/sqrt(2*M_PI*s2) * (steps.back().nScatter + FullCapt);

    // separately integrate each segment to avoid singularities
    for(auto& gs: csegs) y += GSI.apply(gs, E);

    return rate * y;
}
