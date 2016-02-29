/// \file TDynamicHistogram.cc
// This file was produced under the employ of the United States Government,
// and is consequently in the PUBLIC DOMAIN, free from all provisions of
// US Copyright Law (per USC Title 17, Section 105).
// 
// -- Michael P. Mendenhall, 2016

#include "TDynamicHistogram.hh"
#include <cmath>

void TDynamicHistogram::Fill(Double_t x, Double_t w) {
    BinData& b = fDat[FindBin(x)];
    b.sw += w;
    b.sww += w*w;
}

void TDynamicHistogram::Scale(Double_t s) { 
    for(auto& kv: fDat) {
        kv.second.sw *= s;
        kv.second.sww *= s*s;
    }
}

void TDynamicHistogram::Add(const TDynamicHistogram& h, Double_t s, Bool_t rebin) {
    auto const& d = h.GetData();
    if(rebin) {
        for(auto const& kv: d) {
            Int_t b = h.BinCenter(kv.first);
            fDat[b].sw += s*kv.second.sw;
            fDat[b].sww += s*s*kv.second.sww;
        }
    } else {
        for(auto const& kv: d) {
            fDat[kv.first].sw += s*kv.second.sw;
            fDat[kv.first].sww += s*s*kv.second.sww;
        }
    }
}

void TDynamicHistogram::normalize_to_bin_width(Double_t sc) {
    for(auto& kv: fDat) {
        Double_t bw = BinLoEdge(kv.first+1)-BinLoEdge(kv.first);
        kv.second.sw *= sc/bw;
        kv.second.sww *= sc*sc/(bw*bw);
    }
}

TGraphErrors* TDynamicHistogram::MakeGraph() const {
    TGraphErrors* g = new TGraphErrors(fDat.size());
    g->SetTitle(fTitle);
    Int_t n = 0;
    for(auto const& kv: fDat) {
        g->SetPoint(n, BinCenter(kv.first), kv.second.sw);
        g->SetPointError(n, 0, sqrt(kv.second.sww));
        n++;
    }
    return g;
}

void TDynamicHistogram::_Add(const CumulativeData* CD, Double_t s) {
    auto TDH = dynamic_cast<const TDynamicHistogram*>(CD);
    if(TDH) Add(*TDH,s);
}
