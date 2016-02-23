/// \file CumulativeData.hh Virtual base class for objects that can be scaled/summed
// This file was produced under the employ of the United States Government,
// and is consequently in the PUBLIC DOMAIN, free from all provisions of
// US Copyright Law (per USC Title 17, Section 105).
// 
// -- Michael P. Mendenhall, 2016

#ifndef CUMULATIVEDATA_HH
#define CUMULATIVEDATA_HH

#include <Rtypes.h>

/// Virtual base class for objects that can be scaled/summed
class CumulativeData {
public:
    /// Constructor
    CumulativeData() { }
    /// Destructor
    virtual ~CumulativeData() { }
    
    /// Scale contents by factor
    virtual void _Scale(Double_t s) = 0;
    /// add another histogram, assuming same binning convention or re-calculating bins
    virtual void _Add(const CumulativeData* CD, Double_t s = 1.) = 0;
    /// clear data contents
    virtual void _Clear() { _Scale(0); }
};

#endif
