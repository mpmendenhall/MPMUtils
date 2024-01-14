/// @file TRateCategories.hh Chart for event rate of different categories in sequential data
// This file was produced under the employ of the United States Government,
// and is consequently in the PUBLIC DOMAIN, free from all provisions of
// US Copyright Law (per USC Title 17, Section 105).
//
// -- Michael P. Mendenhall, 2016

#ifndef TRATECATEGORIES_HH
#define TRATECATEGORIES_HH

#include <TNamed.h>
#include <TGraphErrors.h>

#include <vector>
#include <array>
#include <map>
using std::vector;
using std::array;
using std::map;
using std::pair;

/// Summary of event rate versus time
class TRateCategories: public TNamed {
public:
    /// Constructor
    TRateCategories(const TString& nm = "", const TString& ttl = "", Double_t dx = 0): TNamed(nm,ttl), fDxMax(dx) { }
    /// Destructor
    virtual ~TRateCategories() { }

    /// data points being summarized
    struct DataPt {
        Int_t c;        ///< point category
        Double_t x;     ///< point time
        Double_t w;     ///< point weigh
    };

    /// Summarized rate data points
    struct SummaryPt {
        Double_t fX = 0;        ///< group x mean value
        Double_t fXX = 0;       ///< group x variance
        Double_t fW = 0;        ///< group sum of weights
    };

    /// Add timed count for category
    void AddPoint(Int_t c, Double_t x, Double_t w = 1.0);
    /// Append another TStripchart
    void Append(const TRateCategories& C);
    /// Set delta x for summarizing points
    void SetDeltaX(Double_t dx) { fDxMax = dx; }

    /// Generate TGraphErrors from linear combination of catgories
    TGraphErrors* MakeGraph(const vector< pair<Int_t,Double_t> >& coeffs, bool per_dx = true) const;

    /// Summarize window contents to datapoint
    void SummarizeWindow();

protected:
    vector<DataPt> fPts;                        ///< points in active window waiting to be summarized
    Double_t fDxMax;                            ///< change in x to trigger archiving of point data
    vector< map<Int_t, SummaryPt> > fDat;       ///< archived summary data

    ClassDef(TRateCategories,1);
};

#endif
