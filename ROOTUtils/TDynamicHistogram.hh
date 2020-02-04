/// \file TDynamicHistogram.hh Histogram with dynamic (sparse) binning
// This file was produced under the employ of the United States Government,
// and is consequently in the PUBLIC DOMAIN, free from all provisions of
// US Copyright Law (per USC Title 17, Section 105).
//
// -- Michael P. Mendenhall, 2016

#ifndef TDYNAMICHISTOGRAM_HH
#define TDYNAMICHISTOGRAM_HH

#include "TCumulative.hh"
#include <TGraphErrors.h>

#include <map>
using std::map;

/// Histogram with dynamic (sparse) binning
class TDynamicHistogram: public TCumulative {
public:
    /// Constructor
    TDynamicHistogram(const TString& nme = "", const TString& ttl = "", Int_t n = 1, Double_t x0 = 0, Double_t x1 = 1):
    TCumulative(nme,ttl), fN(n), fX0(x0), fX1(x1) { }
    /// Destructor
    virtual ~TDynamicHistogram() { }

    /// data in histogram bin
    struct BinData {
        Double_t sw = 0;        ///< sum of weights
        Double_t sww = 0;       ///< sum of weights squared
    };

    /// fill new data point
    void Fill(Double_t x, Double_t w=1.);
    /// scale all bin contents
    void Scale(Double_t s);
    /// add another histogram, assuming same binning convention or re-calculating bins
    void Add(const TDynamicHistogram& h, Double_t s = 1., Bool_t rebin = false);
    /// get data
    const map<Int_t, BinData>& GetData() const { return fDat; }
    /// select bin number
    virtual Int_t FindBin(Double_t x) const { return Int_t(fN*(x-fX0)/(fX1-fX0)); }
    /// position of bin lower edge
    virtual Double_t BinLoEdge(Int_t n) const { return ((fN-n)*fX0 + n*fX1)/fN; }
    /// position of bin center for drawing
    virtual Double_t BinCenter(Int_t n) const { return ((fN-n-0.5)*fX0 + (n+0.5)*fX1)/fN; }
    /// get representation as a TGraphErrors
    TGraphErrors* MakeGraph() const;
    /// divide bins by width and optional additional scaling
    virtual void normalize_to_bin_width(Double_t sc = 1.);

    /// Scale contents by factor
    void _Scale(Double_t s) override { Scale(s); }
    /// add another histogram, assuming same binning convention or re-calculating bins
    void _Add(const CumulativeData* CD, Double_t s = 1.) override;

protected:

    map<Int_t, BinData> fDat;   ///< histogram data
    Double_t fN;                ///< number of bins in prototype interval
    Double_t fX0;               ///< beginning of prototype interval
    Double_t fX1;               ///< end of prototype interval

    ClassDefOverride(TDynamicHistogram,2);
};

#endif
