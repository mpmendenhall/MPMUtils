/// \file GammaEdgeFitter.cc

#include "GammaEdgeFitter.hh"
#include "SmearingIntegral.hh"

GammaEdgeFitter::GammaEdgeFitter(double _E0):
TF1("GammaEdge", this, &GammaEdgeFitter::Evaluate, 0, 1, 7, "GammaEdgeFitter", "Evaluate"),
GammaScatterSteps(_E0, 0), GBG(_E0, 0) {
    SetLineColor(6);
    SetParameter(1, 1);
    lines.emplace_back(1.,1.);

    SetParName(0, "gain");
    SetParName(1, "rate");
    SetParName(2, "nPE");
    SetParName(3, "d");
    SetParName(4, "cb");
    SetParName(5, "kb");
    SetParName(6, "db");

    FixParameter(4, 0);
    FixParameter(5, -1.9);
    FixParameter(6, 0);
}

double GammaEdgeFitter::_eval(double E) {
    // full capture peak
    double dx = E - E0;
    double s2 = E0 / PE_per_MeV;
    double y = exp(-dx*dx/(2*s2))/sqrt(2*M_PI*s2) * (steps.back().nScatter + FullCapt);

    // separately integrate each segment to avoid singularities
    for(const auto& gs: csegs) y += GSI.apply(gs, E);

    return y;
}

double GammaEdgeFitter::Evaluate(double* x, double* p) {
    SigPerE     = p[0];
    rate        = p[1];
    GSI.n_per_x = PE_per_MeV  = p[2];

    // update model for parameter changes
    double _ed = p[3] * get_edens();
    bool do_up = eDens != _ed;

    double _edb = p[6] * get_edens();
    bool do_upb = GBG.eDens != _edb;

    if(do_upb && _edb > 1e-4) GBG.setDens(_edb, nsteps);

    if(do_up) {
        setDens(_ed, nsteps);
        csegs.clear();
        for(const auto& S: steps) csegs.push_back(Egamma_to_Ee(S.EscapeSum));
    }

    if((do_up || do_upb) && _edb) calcRescatter(GBG);

    auto E = *x/SigPerE;
    double y = 0;
    for(const auto& l: lines) y += l.second * _eval(E/l.first) / l.first;

    if(p[4]) {
        y += p[4] * (_edb > 1e-4? GSI.apply(bComptons, E) : 1) * pow(E, p[5]);
    }
    return rate * y;
}

void GammaEdgeFitter::display() const {
    printf("Gamma model fit: %.2f ~ %.2f S/MeV, %.1f ~ %.1f PE/MeV, d = %.2f cm\n",
           GetParameter(0), GetParError(0),
           GetParameter(2), GetParError(2),
           GetParameter(3));
}
