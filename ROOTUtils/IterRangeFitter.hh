/// \file IterRangeFitter.hh Iterative fitting over fit-defined window for ``scale invariant'' fits
// This file was produced under the employ of the United States Government,
// and is consequently in the PUBLIC DOMAIN, free from all provisions of
// US Copyright Law (per USC Title 17, Section 105).
// 
// -- Michael P. Mendenhall, 2016

#ifndef ITERRANGEFITTER_HH
#define ITERRANGEFITTER_HH

#include <TF1.h>
#include <TH1.h>

/// Base class for iterative range fitting
class IterRangeFitter {
public:
    /// Constructor
    IterRangeFitter() { }
    /// Destructor
    virtual ~IterRangeFitter() { delete myF; }

    /// perform fit until range converges
    void doFit(TH1* h, const char* opt = "R");
        
    /// determine range (subclass me!)
    virtual void getRange(double& r0, double& r1) const = 0;
    /// print statement for each step
    virtual void showStep() const;
    
    TF1* myF = NULL;    ///< fit function
    int nmax = 20;      ///< maximum number of iteration attempts
    double rtol = 1e-4; ///< relative tolerance on window edge movement to window width

protected:
    double fr0, fr1;    ///< current fit range
};

/// Gaussian peak iterative-range fitter
class IterRangeGaus: public IterRangeFitter {
public:
    /// Constructor
    IterRangeGaus(double c0, double s0, TF1* f = NULL);
    
    double nsigmalo = 2.;       ///< fit range below peak
    double nsigmahi = 2.;       ///< fit range above peak
    
    /// determine range
    void getRange(double& r0, double& r1) const override;
    /// print statement for each step
    void showStep() const override;
};

/// Erfc edge iterative fitter
class IterRangeErfc: public IterRangeGaus {
public:
    /// Constructor
    IterRangeErfc(double c0, double s0);
};

#endif
