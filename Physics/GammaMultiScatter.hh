/// @file GammaMultiScatter.hh Gamma scattering spectra approximations
// -- Michael P. Mendenhall, LLNL 2022

#ifndef GAMMAMULTISCATTER_HH
#define GAMMAMULTISCATTER_HH

#include "GammaScattering.hh"
#include "TGraphIntegrator.hh"
#include <vector>
using std::vector;

/// Calculator for gamma scattering spectra by numerical integration of cross sections
class GammaScatterSteps {
public:
    /// Constructor
    GammaScatterSteps(double _E0, double _eDens, double _Z = 6, int _npts = 100);
    /// Change electron density; calculate specified number of scatter steps
    void setDens(double _eDens, size_t nsteps = 0);

    double E0;              ///< initial gamma energy
    double eDens;           ///< electron areal density, mol / cm^2
    double Z;               ///< effective Z for photoelectric effect
    int npts;               ///< number of evaluation points

    double Escape_0;        ///< fraction initially escaping
    TGraph Escape;          ///< total escape over all steps except Escape_0 delta-function
    double Scatter_0;       ///< fraction initially scattering down
    double FullCapt;        ///< fully captured gamma fraction (from Photoelectric Effect)
    TGraph gInteract;       ///< Total interaction probability (scatter or Photoelectric) within material vs. energy
    TGraph gCx;             ///< total Compton scattering cross-section vs. gamma energy
    TGraph gPE;             ///< photoelectric effect cross-section
    TSpline3 sPE;           ///< interpolating spline of gPE

    /// Interaction cross sections and probabilities at a given gamma energy
    struct s_Interactions {
        double s_Compt = 0; ///< total Compton cross-section
        double s_PE    = 0; ///< Photoelectric cross-section
        double p_Ixn   = 0; ///< probability of scattering or photoelectric interaction
        double f_Compt = 0; ///< Compton scattering fraction of interactions
    };
    /// Calculate interactions at given energy
    s_Interactions interactionsAt(double E) const;

    /// Gamma information from one scattering step --- use to track total sum e- and gamma out from monoenergetic primaries
    struct s_ScatterStep {
        /// Default Constructor
        s_ScatterStep() { }
        /// Constructor
        s_ScatterStep(const TGraph& gI, double _Emin, double _EmPrev, double _Emax):
        Incident(gI), Emin(_Emin), EminPrev(_EmPrev), Emax(_Emax) { }

        TGraph Incident;    ///< Incident gamma energy distribution [/gamma/MeV]
        TGraph Escape;      ///< Escaped portion of Incident [/gamma/MeV] (=> add to electron scatter energy)
        TGraph EscapeSum;   ///< Escape, in this step's energy range [Emin, EminPrev], contributions summed over all steps

        double nScatter = 0;    ///< integral number re-scattering [/gamma]
        double fullCapt = 0;    ///< Photolectric full energy caputure portion [/gamma]
        double Emin;            ///< lowest gamma energy in incident [MeV]
        double EminPrev;        ///< previous step's minimum energy
        double Emax;            ///< highest gamma energy in incident [MeV], = E0 when from this object's calculations

        TSpline3 _Scatter;  ///< Normalized re-scattering distribution [/gamma/MeV/s_tot(E)] for next calculation stage
    };
    vector<s_ScatterStep> steps;    ///< calculated scattering steps

    /// Perform next scattering calculation step
    void scatter_step();

    /// Transform distribution from escaping gamma to electron energy scale
    TGraph Egamma_to_Ee(const TGraph& g) const;

    /// Resolution-smeared electron spectrum
    TGraph eSpectrum(double PE_per_MeV = 0) const;

    // //////////////////////////// //
    // For "background" contributions
    // from pre-scattered gammas

    /// Electron scattering calculation --- electron deposition from one interaction round
    struct s_eScatterStep {
        /// Constructor
        s_eScatterStep(const TGraph& gI, double _Emin, double _Emax):
        Incident(gI), Emin(_Emin), Emax(_Emax), Ec(compton_edge_e_for_gamma(Emax)) { }

        TGraph Incident;    ///< Incident gamma energy distribution [/gamma/MeV]
        TGraph PhotoElec;   ///< Photoelectric effect electron spectrum [/gamma/MeV], range Emin to Emax
        TSpline3 _Scatter;  ///< Normalized re-scattering distribution [/gamma/MeV/s_tot(E)]

        double Emin;        ///< lowest gamma energy in incident [MeV]
        double Emax;        ///< highest gamma energy in incident [MeV]
        double Ec;          ///< Compton edge electron energy from Emax incident [MeV]

    };
    vector<s_eScatterStep> bSteps;  ///< second-pass scattering
    TGraph bComptons;               ///< Re-scattered Compton electron spectrum [/gamma/MeV]

    /// Calculate re-scattered background contribution escaping from preceding scattering
    void calcRescatter(const GammaScatterSteps& GSS);

    /// Calculate single-scatter spectrum from incident gamma data
    void single_scatter_deposition(s_eScatterStep& S);
    /// One scattering from "degaded" escaping spectrum
    s_eScatterStep fromEscaping(const s_ScatterStep& S);
    /// Compton electrons at energy E produced from pre-calculated _Scatter
    double comptonsFrom(const s_eScatterStep& S, double E);

protected:
    integrator_wrapper scatterIntegrator;   ///< gamma scatter integration calculator
    integrator_wrapper eScatterIntegrator;  ///< electron scatter integration calculator

    /// calculate/update interactions calculation on density change
    void calcIxns();
    /// generate Escape and Scatter curves from Incident
    void splitIncident(s_ScatterStep& S) const;
    /// sum in additional escape step
    void sumEscaped();
};

#endif
