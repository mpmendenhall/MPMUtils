/// @file GammaEdgeFitter.hh Gamma spectum model integrated into ROOT fitter for smeared gamma edge fits
// -- Michael P. Mendenhall, LLNL 2022

#ifndef GAMMAEDGEFITTER
#define GAMMAEDGEFITTER

#include "GammaMultiScatter.hh"
#include "SmearingIntegral.hh"
#include <TF1.h>
using std::pair;

/// Fitter for gamma edge
class GammaEdgeFitter: public TF1, protected GammaScatterSteps {
public:
    /// Contructor
    explicit GammaEdgeFitter(double _E0);

    /// Evaluate with fit parameters
    double Evaluate(double* x, double* p);

    /*
     * Parameters:
     * 0 SigPerE
     * 1 rate
     * 2 PE_per_MeV
     * 3 d effective depth
     * ---- fully empirical background c * S^k
     * 4 c
     * 5 k
     * ---- pre-downscattered component
     * 6 d2 effective degadation depth, cm
    */

    // material properties for "thickness" interpretation
    double e_per_molecule = 10; ///< electrons per molecule unit
    double molar_mass = 18;     ///< molar mass (g)
    double mat_dens = 1;        ///< material density (g/cm^3)

    /// electron density [mol / cm^3]
    double get_edens() const { return mat_dens * e_per_molecule / molar_mass; }
    /// display fit results to stdout
    void display() const;

    /// edge spread into multiple lines [(rel. energy, weight)] under same-shape approx
    vector<pair<double,double>> lines;

protected:
    int nsteps = 10;            ///< number of scattering steps
    double SigPerE = 1.;        ///< signal per energy scaling
    double rate = 1.;           ///< rate scaling factor
    double PE_per_MeV = 400;    ///< energy resolution

    GammaScatterSteps GBG;      ///< re-scattered degraded background calculation

    vector<TGraph> csegs;       ///< electron spectrum segments
    gaussian_smearing_integral GSI; ///< resolution smearer

    /// internal unscaled evaluation at point
    double _eval(double E);
};

#endif
