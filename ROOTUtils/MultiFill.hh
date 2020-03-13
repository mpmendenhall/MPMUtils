/// \file MultiFill.hh Histogram + covariance matrix for correlated multiple-bin fills
// Michael P. Mendenhall, LLNL 2020

#ifndef MULTIFILL_HH
#define MULTIFILL_HH

#include "CumulativeData.hh"
#include <TDirectory.h>
#include <TH1.h>
#include <TMatrixD.h>
#include <stdexcept>
#include <TH2F.h>
#include <vector>
using std::vector;

/// Covariance matrix paired to histogram for correlated-bin fills
class MultiFill: public CumulativeData {
public:
    /// Constructor, corresponding to histogram
    MultiFill(const string& _name, TH1& H);
    /// Constructor, loaded from file
    MultiFill(const string& _name, TDirectory& d);
    /// Destructor
    ~MultiFill() { delete M; }

    /// (unity weights) fill by bin numbers
    template<class V>
    void fillBins(const V& vb) {
        for(auto b1: vb) {
            h->AddBinContent(b1);
            for(auto b2: vb) ++(*M)(b1, b2);
        }
    }

    /// (unity weights) fill from iterable values
    template<class V>
    void fill(const V& v) {
        vector<int> vb;
        for(auto x: v) vb.push_back(h->FindBin(x));
        fillBins(vb);
    }

    /// scaling of M only --- assumes h is managed externally
    void Scale(double s) override { *M *= s*s; }
    /// scaling, including h
    void Scaleh(double s) { Scale(s); h->Scale(s); }
    /// addition of M only --- assumes h is managed externally
    void Add(const CumulativeData& CD, double s = 1.) override { *M += *dynamic_cast<const MultiFill&>(CD).M * (s * s); }
    /// addition including h
    void Addh(const CumulativeData& CD, double s = 1.) { Add(CD,s); h->Add(dynamic_cast<const MultiFill&>(CD).h, s); }
    /// Store state (M only)
    void Write() override { checkInit(); M->Write((name+"_Cov").c_str()); }
    /// divide bins by width (optional additional scaling)
    void normalize_to_bin_width(double xscale = 1., const string& ytitle = "");
    /// Overwrite histogram errorbars from covariance diagonal
    void diagErrors();
    /// Plottable covariance matrix as TH2F (caller accepts memory management responsibility)
    TH2F* covHist() const;

    TH1* h = nullptr;       ///< the histogram (assumed to be managed externally)
    TMatrixD* M = nullptr;  ///< covariance matrix (owned by this)
protected:
    /// initialization/usability check
    virtual void checkInit() const { if(!h || !M) throw std::logic_error("MultiFill uninitialized"); };
};

#endif
