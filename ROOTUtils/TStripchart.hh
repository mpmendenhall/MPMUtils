/// \file TStripchart.hh ``Strip chart'' recorder for sequential data
// This file was produced under the employ of the United States Government,
// and is consequently in the PUBLIC DOMAIN, free from all provisions of
// US Copyright Law (per USC Title 17, Section 105).
//
// -- Michael P. Mendenhall, 2016

#ifndef TSTRIPCHART_HH
#define TSTRIPCHART_HH

#include <TNamed.h>
#include <TGraphErrors.h>

#include <vector>
#include <array>
using std::vector;
using std::array;

/// Summary of data versus time
class TStripchart: public TNamed {
public:
    /// Constructor
    TStripchart(const TString& nm = "", const TString& ttl = "", Double_t dx = 0): TNamed(nm,ttl), fDxMax(dx) { }
    /// Destructor
    virtual ~TStripchart() { }

    /// Summarized data points averaging several readings
    struct SummaryPt {
        Double_t fX = 0;                ///< group x mean value
        Double_t fXX = 0;               ///< group x variance
        Double_t fW = 0;                ///< group sum of weights
        Double_t fY = 0;                ///< group y mean value
        Double_t fYY = 0;               ///< group y variance
    };

    /// Add data point
    void AddPoint(Double_t x, Double_t y, Double_t w = 1.0);
    /// Append another TStripchart
    void Append(const TStripchart& C);
    /// Set delta x for summarizing points
    void SetDeltaX(Double_t dx) { fDxMax = dx; }
    /// Get data points
    const vector<SummaryPt>& GetData() const { return fDat; }

    /// Convert contents to TGraphErrors for display
    TGraphErrors* MakeGraph() const;
    /// Convert weights to event rate graph (optionally dividing out time interval fDxMax)
    TGraphErrors* MakeRateGraph(bool per_dt = false) const;

    /// Summarize window contents to datapoint
    void SummarizeWindow();

protected:

    vector< array<Double_t,3> > fPts;   ///< points in active window waiting to be summarized
    Double_t fSw = 0;                   ///< sum of weights in current window
    Double_t fDxMax;                    ///< change in x to trigger archiving of point data
    vector<SummaryPt> fDat;             ///< archived summary data

    ClassDef(TStripchart,1);
};

#endif
