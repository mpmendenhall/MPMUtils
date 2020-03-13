/// \file MultiFill.cc

#include "MultiFill.hh"
#include <TAxis.h>
#include <cmath>
#include <vector>
using std::vector;

MultiFill::MultiFill(const string& _name, TH1& H):
CumulativeData(_name), h(&H), M(new TMatrixD(H.GetNcells(), H.GetNcells())) { }

MultiFill::MultiFill(const string& _name, TDirectory& d):
CumulativeData(_name),
h(dynamic_cast<TH1*>(d.Get(name.c_str()))),
M(dynamic_cast<TMatrixD*>(d.Get((name + "_Cov").c_str()))) {
    if(!h) throw std::runtime_error("Missing MultiFill histogram '"+name+"'");
    if(!M) throw std::runtime_error("Missing MultiFill covariance '"+name+"_Cov'");
}

void MultiFill::normalize_to_bin_width(double xscale, const string& ytitle) {
    checkInit();
    if(xscale != 1.) Scaleh(xscale);

    Int_t bx1,bx2,by,bz;
    TAxis* A = h->GetXaxis();

    for(int i=0; i<h->GetNcells(); ++i) {
        h->GetBinXYZ(i, bx1, by, bz);
        double s1 = (bx1 > 0 && bx1 <= A->GetNbins())? 1./A->GetBinWidth(bx1) : 1;
        h->SetBinContent(i, h->GetBinContent(i)*s1);
        h->SetBinError(i, h->GetBinError(i)*s1);

        for(int j=0; j<h->GetNcells(); ++j) {
            h->GetBinXYZ(j,bx2,by,bz);
            double s2 = (bx2 > 0 && bx2 <= A->GetNbins())? 1./A->GetBinWidth(bx2) : 1;
            (*M)(i,j) *= s1*s2;
        }
    }

    if(ytitle.size()) h->GetYaxis()->SetTitle(ytitle.c_str());
}

void MultiFill::diagErrors() {
    checkInit();
    for(int i=0; i<h->GetNcells(); ++i) h->SetBinError(i, sqrt((*M)(i,i)));
}

TH2F* MultiFill::covHist() const {
    auto nc = h->GetNcells();
    bool is1D = nc == h->GetNbinsX() + 2;
    TH2F* hh;

    if(is1D) {
        vector<Double_t> xbl(nc-1);
        auto ax = h->GetXaxis();
        ax->GetLowEdge(xbl.data());
        xbl.back() = ax->GetBinUpEdge(nc-2);

        hh = new TH2F((name + "_Cov_h").c_str(),
                      (name + " Covariance").c_str(),
                      nc-2, xbl.data(), nc-2, xbl.data());
        hh->GetXaxis()->SetTitle(ax->GetTitle());
        hh->GetYaxis()->SetTitle(ax->GetTitle());

    } else {
        hh = new TH2F((name + "_Cov_h").c_str(),
                      (name + " Covariance").c_str(),
                      nc, 0, nc, nc, 0, nc);
    }

    hh->GetZaxis()->SetTitle("Covariance");

    for(int i=0; i<nc; ++i) {
        for(int j=0; j<nc; ++j) {
            hh->SetBinContent(hh->GetBin(i + !is1D, j + !is1D), (*M)(i,j));
        }
    }

    return hh;
}
