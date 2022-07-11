/// \file GammaMultiScatter.hh Gamma scattering spectra approximations
// -- Michael P. Mendenhall, LLNL 2022

#ifndef GAMMAMULTISCATTER_HH
#define GAMMAMULTISCATTER_HH

#include "GammaScattering.hh"
#include <TGraph.h>
#include <TSpline.h>
#include <vector>
using std::vector;

class GammaScatterSteps {
public:
    /// Constructor
    GammaScatterSteps(double _E0, double _eDens, int _npts = 100);

    double E0;              ///< initial gamma energy
    double eDens;           ///< electron areal density, mol / cm^2
    int npts;               ///< number of evaluation points

    double Escape_0;        ///< fraction initially escaping
    TGraph Escape;          ///< total escape over all steps except Escape_0 delta-function
    double Scatter_0;       ///< fraction initially scattering down
    TGraph gEsc;            ///< Escape probability vs. energy
    TGraph gCx;             ///< total scattering cross-section vs. gamma energy

    /// Information from one scattering step
    struct s_ScatterStep {
        TGraph Incident;    ///< Incident gamma energy distribution [/gamma/MeV]
        TGraph Escape;      ///< Escaped portion of Incident [/gamma/MeV] (=> add to electron scatter energy)
        TGraph Scatter;     ///< Normalized re-scattering distribution [/gamma/MeV/s_tot(E)] for next calculation stage
        TSpline3 sScatter;  ///< Splined version of Scatter for integration
        double nScatter;    ///< integral number re-scattering [/gamma]
        double Emin;        ///< lowest gamma energy in incident
    };
    vector<s_ScatterStep> steps;    ///< calculated scattering steps

    /// Perform next scattering calculation step
    void scatter_step();

    /// Transform distribution from escaping gamma to electron energy scale
    TGraph Egamma_to_Ee(const TGraph& g) const;

protected:
    /// generate Escape and Scatter curves from Incident
    void splitIncident();
    /// sum in additional escape step
    void sumEscaped();

};


/// Helper for integrating TGraph
struct tgraph_integrator {
    /// Constructor
    explicit tgraph_integrator(const TGraph& _g, size_t _n = 0):
    g(_g), nadaptive(_n) { }

    const TGraph& g;    ///< graph to be integrated

    size_t nadaptive;   ///< adaptive integration intervals (0 to disable)
    size_t neval = 0;   ///< returned number of evaluation points
    double res = 0;     ///< returned integral result
    double abserr = 0;  ///< returned error estimate

    /// perform integration
    double integrate(double x0, double x1, double epsab = 1e-4, double epsrel = 1e-3);
};

#endif
