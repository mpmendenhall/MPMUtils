/// \file TStripchart.cc
// This file was produced under the employ of the United States Government,
// and is consequently in the PUBLIC DOMAIN, free from all provisions of
// US Copyright Law (per USC Title 17, Section 105).
//
// -- Michael P. Mendenhall, 2016

#include "TStripchart.hh"

void TStripchart::AddPoint(Double_t x, Double_t y, Double_t w) {
    if(fPts.size() && !(fabs(x - fPts[0][0]) <= fDxMax)) SummarizeWindow();
    fSw += w;
    array<Double_t,3> a = { {x,y,w} };
    fPts.push_back(a);
}

void TStripchart::SummarizeWindow() {
    if(!fPts.size()) return;

    SummaryPt P;
    P.fW = fSw;
    for(auto const& a: fPts) {
        P.fX += a[0]*a[2];
        P.fY += a[1]*a[2];
    }
    P.fX /= fSw;
    P.fY /= fSw;

    for(auto const& a: fPts) {
        P.fXX += (a[0]-P.fX)*(a[0]-P.fX)*a[2];
        P.fYY += (a[1]-P.fY)*(a[1]-P.fY)*a[2];
    }
    P.fXX /= fSw;
    P.fYY /= fSw;

    fDat.push_back(P);
    fSw = 0;
    fPts.clear();
}

void TStripchart::Append(const TStripchart& C) {
    SummarizeWindow();
    fDat.insert(fDat.end(), C.fDat.begin(), C.fDat.end());
    for(auto const& a: C.fPts) AddPoint(a[0], a[1], a[2]);
}

TGraphErrors* TStripchart::MakeGraph() const {
    TGraphErrors* g = new TGraphErrors(fDat.size());
    for(size_t i=0; i<fDat.size(); i++) {
        auto const& P = fDat[i];
        g->SetPoint(i,P.fX,P.fY);
        g->SetPointError(i,sqrt(P.fXX),sqrt(P.fYY));
    }
    return g;
}

TGraphErrors* TStripchart::MakeRateGraph(bool per_dt) const {
    TGraphErrors* g = new TGraphErrors(fDat.size());
    for(size_t i=0; i<fDat.size(); i++) {
        auto const& P = fDat[i];
        double y = P.fW;
        double dy = sqrt(y);
        if(per_dt) { y /= fDxMax; dy /= fDxMax; }
        g->SetPoint(i,P.fX,y);
        g->SetPointError(i,sqrt(P.fXX),dy);
    }
    return g;
}
