/// \file TGraphIntegrator.hh Helper for integrating TGraph using gsl intgration functions
// Michael P. Mendenhall, 2022

#ifndef TGRAPHINTEGRATOR_HH
#define TGRAPHINTEGRATOR_HH

#include <TGraph.h>
#include <TSpline.h>
#include <gsl/gsl_integration.h>

/// gsl integration options
class _integrator_info {
public:
    /// Constructor
    explicit _integrator_info(size_t _n = 0): nadaptive(_n) {
        if(nadaptive) wi = gsl_integration_workspace_alloc(nadaptive);
    }
    /// Destructor
    ~_integrator_info() { if(wi) gsl_integration_workspace_free(wi); }

    size_t neval = 0;       ///< returned number of evaluation points
    double res = 0;         ///< returned integral result
    double abserr = 0;      ///< returned error estimate
    double epsab = 1e-4;    ///< requested absolute error bound
    double epsrel = 1e-3;   ///< requested relative error bound

    /// perform integration
    double integrate(double x0, double x1);

protected:
    size_t nadaptive = 0;   ///< adaptive integration intervals (0 to disable)
    gsl_function f;         ///< integration function
    gsl_integration_workspace* wi = nullptr;    ///< adaptive intgration workspace
};

/// Integrate TGraph with GSL funcions
class tgraph_integrator: public _integrator_info {
public:
    /// Constructor
    explicit tgraph_integrator(const TGraph& _g, size_t _n = 0);

    const TGraph& g;    ///< graph to be integrated
};

/// Integrate TSpline with GSL funcions
class tspline_integrator: public _integrator_info {
public:
    /// Constructor
    explicit tspline_integrator(const TSpline& _s, size_t _n = 0);

    const TSpline& s;    ///< spline to be integrated
};

#endif
