/// \file TStripchart.cc
// This file was produced under the employ of the United States Government,
// and is consequently in the PUBLIC DOMAIN, free from all provisions of
// US Copyright Law (per USC Title 17, Section 105).
//
// -- Michael P. Mendenhall, 2016

#include "TRatechart.hh"

void TRatechart::AddPoint(Double_t x, Double_t w)  {
    if(fPts.size() && !(fabs(x - fPts[0][0]) <= fDxMax)) SummarizeWindow();
    fSw += w;
    array<Double_t,2> a = { {x,w} };
    fPts.push_back(a);
}

void TRatechart::Append(const TRatechart& C) {
    SummarizeWindow();
    fDat.insert(fDat.end(), C.fDat.begin(), C.fDat.end());
    for(auto const& a: C.fPts) AddPoint(a[0], a[1]);
}

TGraphErrors* TRatechart::MakeGraph(bool per_dt, Double_t xscale) const {
    TGraphErrors* g = new TGraphErrors(fDat.size());
    for(size_t i=0; i<fDat.size(); i++) {
        auto const& P = fDat[i];
        double y = P.fW;
        double dy = sqrt(y);
        if(per_dt) { y /= fDxMax; dy /= fDxMax; }
        g->SetPoint(i,P.fX,y*xscale);
        g->SetPointError(i,sqrt(P.fXX),dy*xscale);
    }
    return g;
}

void TRatechart::SummarizeWindow()  {
    if(!fPts.size()) return;

    SummaryPt P;
    P.fW = fSw;
    for(auto const& a: fPts) P.fX += a[0]*a[1];
    P.fX /= fSw;

    for(auto const& a: fPts) P.fXX += (a[0]-P.fX)*(a[0]-P.fX)*a[1];
    P.fXX /= fSw;

    fDat.push_back(P);
    fSw = 0;
    fPts.clear();
}
