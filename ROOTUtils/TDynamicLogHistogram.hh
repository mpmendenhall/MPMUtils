/// \file TDynamicLogHistogram.hh Histogram with logarithmic dynamic binning
// This file was produced under the employ of the United States Government,
// and is consequently in the PUBLIC DOMAIN, free from all provisions of
// US Copyright Law (per USC Title 17, Section 105).
// 
// -- Michael P. Mendenhall, 2016

#ifndef TDYNAMICLOGHISTOGRAM_HH
#define TDYNAMICLOGHISTOGRAM_HH

#include "TDynamicHistogram.hh"
#include <cmath>

/// Histogram with dynamic (sparse) binning
class TDynamicLogHistogram: public TDynamicHistogram {
public:
    /// Constructor
    TDynamicLogHistogram(const TString& nme = "", const TString& ttl = "", Int_t n = 1, Double_t x0 = 0, Double_t x1 = 1):
    TDynamicHistogram(nme,ttl,n,x0,x1) { }
    /// Destructor
    virtual ~TDynamicLogHistogram() { }

    /// select bin number
    virtual Int_t FindBin(Double_t x) const { return Int_t(fN*log(x/fX0)/log(fX1/fX0)); }
    /// position of bin lower edge
    virtual Double_t BinLoEdge(Int_t n) const { return fX0*pow(fX1/fX0, Double_t(n)/fN); }
    /// position of bin center for drawing
    virtual Double_t BinCenter(Int_t n) const { return fX0*pow(fX1/fX0, (n+0.5)/fN); }
    
    ClassDef(TDynamicLogHistogram,1);
};

#endif
