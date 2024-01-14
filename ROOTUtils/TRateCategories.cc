/// @file TRateCategories.cc
// This file was produced under the employ of the United States Government,
// and is consequently in the PUBLIC DOMAIN, free from all provisions of
// US Copyright Law (per USC Title 17, Section 105).
//
// -- Michael P. Mendenhall, 2016

#include "TRateCategories.hh"

void TRateCategories::AddPoint(Int_t c, Double_t x, Double_t w) {
    if(fPts.size() && !(fabs(x - fPts[0].x) <= fDxMax)) SummarizeWindow();
    DataPt P;
    P.c = c;
    P.x = x;
    P.w = w;
    fPts.push_back(P);
}

void TRateCategories::Append(const TRateCategories& C) {
    SummarizeWindow();
    fDat.insert(fDat.end(), C.fDat.begin(), C.fDat.end());
    for(auto const& P: C.fPts) AddPoint(P.c, P.x, P.w);
}

TGraphErrors* TRateCategories::MakeGraph(const vector< pair<Int_t,Double_t> >& coeffs, bool per_dx) const {
    TGraphErrors* g = new TGraphErrors();

    int n = 0;
    for(auto const& m: fDat) {
        double sxw = 0;
        double sxxw = 0;
        double sw = 0;
        double sw0w = 0;
        double sw0w0w = 0;
        for(auto c: coeffs) {
            auto it = m.find(c.first);
            if(it==m.end()) continue;

            Double_t w = it->second.fW;
            Double_t w0 = c.second;

            sw += w;
            sw0w += w0 * w;
            sw0w0w += w0 * w0 * w;
            sxw += it->second.fX * w;
            sxxw += it->second.fXX * w;
        }
        if(!sw) continue;

        g->SetPoint(n, sxw/sw, sw0w*(per_dx? 1./fDxMax : 1));
        g->SetPointError(n, sqrt(sxxw/sw), sqrt(sw0w0w)*(per_dx? 1./fDxMax : 1));
        n++;
    }

    return g;
}

void TRateCategories::SummarizeWindow() {
    if(!fPts.size()) return;

    map<Int_t, SummaryPt> Ps;

    for(auto const& a: fPts) {
        Ps[a.c].fW += a.w;
        Ps[a.c].fX += a.w * a.x;
    }
    for(auto& kv: Ps) kv.second.fX /= kv.second.fW;

    for(auto const& a: fPts) Ps[a.c].fXX += (a.x-Ps[a.c].fX)*(a.x - Ps[a.c].fX)*a.w;
    for(auto& kv: Ps) kv.second.fXX /= kv.second.fW;

    fDat.push_back(Ps);
    fPts.clear();
}
