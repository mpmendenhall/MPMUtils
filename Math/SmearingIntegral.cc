/// \File SmearingIntegral.cc

#include "SmearingIntegral.hh"
#include <cmath>
#include <gsl/gsl_integration.h>

double _eval_smeared(double x, void* p) {
    auto& S = *static_cast<gaussian_smearing_integral*>(p);
    if(!x) return 0;
    double dx = x - S.x;
    double s2 = x / S.n_per_x;
    return exp(-dx*dx/(2*s2))/sqrt(2*M_PI*s2) * S.g->Eval(x);
}

gaussian_smearing_integral::gaussian_smearing_integral(double _n):
integrator_wrapper(100), n_per_x(_n) {
    f.params = this;
    f.function = &_eval_smeared;
}

double gaussian_smearing_integral::apply(const TGraph& _g, double _x) {
    double x0 = _g.GetX()[0];
    if(!_x) return x0 == 0? _g.Eval(0) : 0;

    x = _x;
    g = &_g;
    double s = integrate(x0, g->GetX()[g->GetN()-1]);
    g = nullptr;
    return s;
}
