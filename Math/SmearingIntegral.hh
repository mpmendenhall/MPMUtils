/// \File SmearingIntegral.hh Apply energy-resolution-smearing response to distributions
// Michael P. Mendenhall, LLNL 2022

#ifndef SMEARINGINTEGRAL_HH
#define SMEARINGINTEGRAL_HH

#include "TGraphIntegrator.hh"

/// Calculator for performing gaussian sqrt(n) smearing calculation
class gaussian_smearing_integral: protected integrator_wrapper {
public:
    /// Constructor
    explicit gaussian_smearing_integral(double _n = 1.);

    double n_per_x;         ///< "statistical counts" per x unit
    double x = 0;           ///< current evaluation point x
    TGraph const* g = nullptr;  ///< current evaluation graph

    /// Value at x of g smeared by sigma = sqrt(x * n_per_x)
    double apply(const TGraph& _g, double _x);
};

#endif
