/// \file TRatechart.hh Chart for event rate in sequential data
// This file was produced under the employ of the United States Government,
// and is consequently in the PUBLIC DOMAIN, free from all provisions of
// US Copyright Law (per USC Title 17, Section 105).
// 
// -- Michael P. Mendenhall, 2016

#ifndef TRATECHART_HH
#define TRATECHART_HH

#include <TNamed.h>
#include <TGraphErrors.h>

#include <vector>
#include <array>
using std::vector;
using std::array;

/// Summary of event rate versus time
class TRatechart: public TNamed {
public:    
    /// Constructor
    TRatechart(const TString& nm = "", const TString& ttl = "", Double_t dx = 0): TNamed(nm,ttl), fDxMax(dx) { }
    /// Destructor
    virtual ~TRatechart() { }
    
    /// Summarized data points averaging several readings
    struct SummaryPt {
        Double_t fX = 0;                ///< group x mean value
        Double_t fXX = 0;               ///< group x variance
        Double_t fW = 0;                ///< group sum of weights
    };
    
    /// Add data point
    void AddPoint(Double_t x, Double_t w = 1.0);
    /// Append another TStripchart
    void Append(const TRatechart& C);
    /// Set delta x for summarizing points
    void SetDeltaX(Double_t dx) { fDxMax = dx; }
    /// Get data points
    const vector<SummaryPt>& GetData() const { return fDat; }
    
    /// Convert contents to TGraphErrors for display
    TGraphErrors* MakeGraph(bool per_dt = true, Double_t xscale = 1.0) const;
    
    /// Summarize window contents to datapoint
    void SummarizeWindow();
    
protected:
    
    vector< array<Double_t,2> > fPts;   ///< points in active window waiting to be summarized
    Double_t fSw = 0;                   ///< sum of weights in current window
    Double_t fDxMax;                    ///< change in x to trigger archiving of point data
    vector<SummaryPt> fDat;             ///< archived summary data
    
    ClassDef(TRatechart,1);
};

#endif
