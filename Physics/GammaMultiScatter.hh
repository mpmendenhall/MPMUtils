/// \file GammaMultiScatter.hh Gamma scattering spectra approximations
// -- Michael P. Mendenhall, LLNL 2022

#ifndef GAMMAMULTISCATTER_HH
#define GAMMAMULTISCATTER_HH

#include "GammaScattering.hh"
#include "TGraphIntegrator.hh"
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
    TSpline3 sEsc;          ///< Interpolating spline of gEsc
    TGraph gCx;             ///< total scattering cross-section vs. gamma energy
    TSpline3 sCx;           ///< interpolating spline of gCx

    /// Information from one scattering step
    struct s_ScatterStep {
        /// Default Constructor
        s_ScatterStep() { }
        /// Constructor
        s_ScatterStep(const TGraph& gI, double _Emin): Incident(gI), Emin(_Emin) { }

        TGraph Incident;    ///< Incident gamma energy distribution [/gamma/MeV]
        TGraph Escape;      ///< Escaped portion of Incident [/gamma/MeV] (=> add to electron scatter energy)
        TGraph EscapeSum;   ///< Escaped portion of Incident, summed over all steps in step energy range
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

    /// Resolution-smeared electron spectrum
    TGraph eSpectrum(double PE_per_MeV = 0) const;

protected:
    /// generate Escape and Scatter curves from Incident
    void splitIncident();
    /// sum in additional escape step
    void sumEscaped();

};

#endif
