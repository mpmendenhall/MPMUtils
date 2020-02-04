/// \file TCumulative.hh Virtual base class combining TNamed and CumulativeData
// This file was produced under the employ of the United States Government,
// and is consequently in the PUBLIC DOMAIN, free from all provisions of
// US Copyright Law (per USC Title 17, Section 105).
//
// -- Michael P. Mendenhall, 2016

#ifndef TCUMULATIVE_HH
#define TCUMULATIVE_HH

#include "CumulativeData.hh"
#include <TNamed.h>

/// Virtual base class for TNamed objects that can be summed (with like derived classes) and scaled
class TCumulative: public TNamed, public CumulativeData {
public:
    /// Constructor
    TCumulative(const TString& nme = "", const TString& ttl = ""): TNamed(nme,ttl) { }
    /// Destructor
    virtual ~TCumulative() { }

    ClassDefOverride(TCumulative,1);
};

#endif
