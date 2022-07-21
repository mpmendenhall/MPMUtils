/// \file GammaEdgeFitter.hh Gamma spectum model integrated into ROOT fitter for smeared gamma edge fits
// -- Michael P. Mendenhall, LLNL 2022

#ifndef GAMMAEDGEFITTER
#define GAMMAEDGEFITTER

#include "GammaMultiScatter.hh"
#include <TF1.h>
#include <array>
using std::array;

class GammaEdgeFitter: public TF1, protected GammaScatterSteps {
public:
    /// Contructor
    GammaEdgeFitter(double _E0);

    /// Evaluate with fit parameters
    double Evaluate(double* x, double* p);

    double SigPerE = 1.;        ///< signal per energy scaling
    double rate = 1.;           ///< rate scaling factor
    double PE_per_MeV = 400;    ///< energy resolution

protected:
    /// Update unsmeared model calculation
    void UpdateCore();

    vector<TGraph> csegs;       ///< electron spectrum segments
};

#endif
