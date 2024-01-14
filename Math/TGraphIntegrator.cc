/// @file TGraphIntegrator.cc

#include "TGraphIntegrator.hh"

double _eval_tgraph(double x, void* p) { return static_cast<const TGraph*>(p)->Eval(x); }
double _eval_tspline(double x, void* p) { return static_cast<const TSpline*>(p)->Eval(x); }

double integrator_wrapper::integrate(double x0, double x1) {
    if(nadaptive) {
        gsl_integration_qags(&f, x0, x1, epsab, epsrel, nadaptive, wi, &res, &abserr);
    } else {
        gsl_integration_qng(&f, x0, x1, epsab, epsrel, &res, &abserr, &neval);
    }
    return res;
}

tgraph_integrator::tgraph_integrator(const TGraph& _g, size_t _n):
integrator_wrapper(_n), g(_g) {
    setParams((void*)(&g));
    f.function = &_eval_tgraph;
}

tspline_integrator::tspline_integrator(const TSpline& _s, size_t _n):
integrator_wrapper(_n), s(_s) {
    setParams((void*)(&s));
    f.function = &_eval_tspline;
}
