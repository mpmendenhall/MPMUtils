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
    TCumulative(const TString& nme = "", const TString& ttl = ""): TNamed(nme,ttl), CumulativeData((const char*)(nme)) { }
    /// Destructor
    virtual ~TCumulative() { }
    /// Clear contents
    void Clear(const char* o = 0) override { TNamed::Clear(o); Scale(0); }

    ClassDefOverride(TCumulative, 2);
};

#endif
