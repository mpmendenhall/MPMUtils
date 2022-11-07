/// \file GraphicsUtils.cc
#include "GraphicsUtils.hh"
#include "PathUtils.hh"
#include "StringManip.hh"
#include <cassert>

#include <unistd.h>

#include <TPad.h>
#include <TCanvas.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TLatex.h>
#include <TBox.h>
#include <TColor.h>

bool compareHistosByMax(TH1* i, TH1* j) {
    assert(i && j);
    return i->GetMaximum() < j->GetMaximum();
}

double getXmin(const TH1* h) { assert(h); return h->GetBinLowEdge(1); }
double getXmax(const TH1* h) { assert(h); return h->GetBinLowEdge(h->GetNbinsX()+1); }

bool compareHistosByXmin(TH1* i, TH1* j) {
    assert(i && j);
    return getXmin(i) < getXmin(j);
}
bool compareHistosByXmax(TH1* i, TH1* j) {
    assert(i && j);
    return getXmax(i) < getXmax(j);
}

double drawSimulHistos(vector<TH1*>& hists, const string& opt, const string& newTitle) {
    if(!hists.size())
        return 0;
    printf("Drawing %i histograms together",(int)hists.size()); fflush(stdout);
    double xmin = getXmin(*std::min_element(hists.begin(),hists.end(),compareHistosByXmin));
    printf(" spanning x=(%g,",xmin); fflush(stdout);
    double xmax = getXmax(*std::max_element(hists.begin(),hists.end(),compareHistosByXmax));
    printf("%g), ",xmax); fflush(stdout);
    TH1* maxHist = *std::max_element(hists.begin(),hists.end(),compareHistosByMax);
    assert(maxHist);
    printf("with ymax = %g...",maxHist->GetMaximum()); fflush(stdout);
    //maxHist->SetAxisRange(xmin,xmax,"X");
    string oldTitle = maxHist->GetTitle();
    if(newTitle != "DEFAULT")
        maxHist->SetTitle(newTitle.c_str());
    maxHist->Draw(opt.c_str());
    for(auto h: hists) {
        assert(h);
        if(h == maxHist)
            continue;
        if(opt.size())
            h->Draw((opt+" SAME").c_str());
        else
            h->Draw("SAME");
    }
    printf(" Done.\n");

            maxHist->SetTitle(oldTitle.c_str());
            return maxHist->GetMaximum();
}

void drawHistoPair(TH1* hRed, TH1* hBlue, const string& opt, Int_t c1, Int_t c2) {
    assert(hRed && hBlue);
    hRed->SetLineColor(c1);
    hRed->SetMarkerColor(c1);
    hBlue->SetLineColor(c2);
    hBlue->SetMarkerColor(c2);
    vector<TH1*> hToPlot;
    hToPlot.push_back(hRed);
    hToPlot.push_back(hBlue);
    drawSimulHistos(hToPlot,opt);
}

void drawDataMCPair(TH1* dat, TH1* mc) {
    dat->SetLineColor(1);
    mc->SetMarkerStyle(33);
    mc->SetMarkerColor(1);
    if(dat->GetMaximum() > mc->GetMaximum()) {
        dat->Draw("H E0");
        mc->Draw("P SAME");
    } else {
        mc->Draw("P");
        dat->Draw("H E0 SAME");
    }
}

TEllipse* drawCircle(float r, Int_t color, Int_t lstyle, float x0, float y0) {
    TEllipse* e = new TEllipse(x0,y0,r,r);
    e->SetFillStyle(0);
    e->SetLineColor(color);
    e->SetLineStyle(lstyle);
    e->Draw();
    return e;
}

TPolyLine* makeEllipse(float x0, float y0, const double* iSigma) {
    int npts = 50;
    vector<Double_t> xs;
    vector<Double_t> ys;
    for(int i=0; i<npts; i++) {
        double th = i*2*M_PI/(npts-1);
        double c = cos(th);
        double s = sin(th);
        double r = 1./sqrt((iSigma[0]*c+iSigma[2]*s)*c + (iSigma[1]*c+iSigma[3]*s)*s);
        xs.push_back(r*c+x0);
        ys.push_back(r*s+y0);
    }
    return new TPolyLine(xs.size(), &xs[0], &ys[0]);
}

TLine* drawVLine(Float_t x, TVirtualPad* C, Int_t color, Int_t style) {
    if(!C) C = gPad;
    Double_t xmin,ymin,xmax,ymax;
    C->Update();
    C->GetRangeAxis(xmin,ymin,xmax,ymax);
    if(C->GetLogy()) {
        ymin = pow(10,ymin);
        ymax = pow(10,ymax);
    }
    TLine* l = new TLine(x,ymin,x,ymax);
    l->SetLineColor(color);
    l->SetLineStyle(style);
    l->Draw();
    return l;
}

TLine* drawHLine(Float_t y, TVirtualPad* C, Int_t color, Int_t style) {
    if(!C) C = gPad;
    Double_t xmin,ymin,xmax,ymax;
    C->Update();
    C->GetRangeAxis(xmin,ymin,xmax,ymax);
    if(C->GetLogx()) {
        xmin = pow(10,xmin);
        xmax = pow(10,xmax);
    }
    TLine* l = new TLine(xmin,y,xmax,y);
    l->SetLineColor(color);
    l->SetLineStyle(style);
    l->Draw();
    return l;
}

void drawExcludedRegion(Float_t x0, Float_t x1, TCanvas* C, Int_t color, Int_t fill) {
    Double_t xmin,ymin,xmax,ymax;
    C->Update();
    C->GetRangeAxis(xmin,ymin,xmax,ymax);
    if(C->GetLogy()) {
        ymin = pow(10,ymin);
        ymax = pow(10,ymax);
    }
    TBox* r = new TBox(x0,ymin,x1,ymax);
    r->SetFillColor(color);
    r->SetFillStyle(fill);
    r->Draw();
}

void makeGrayscalepalette(bool b2w) {
    Double_t l[2] = { double(!b2w), double(b2w) };
    Double_t s[2] = { 0., 1. };
    TColor::CreateGradientColorTable(2, s, l, l, l, 255);
}

void makeRBpalette() {
    const Int_t NRGBs = 5;
    const Int_t NCont = 255;

    Double_t stops[NRGBs] = { 0.00, 0.25, 0.50, 0.75, 1.00 };
    Double_t red[NRGBs]   = { 0.00, 0.00, 1.00, 0.75, 1.00 };
    Double_t green[NRGBs] = { 0.00, 0.25, 1.00, 0.00, 0.80 };
    Double_t blue[NRGBs]  = { 1.00, 0.50, 1.00, 0.00, 0.00 };
    TColor::CreateGradientColorTable(NRGBs, stops, red, green, blue, NCont);
    gStyle->SetNumberContours(NCont);
}

void shiftZaxis(TH2& h, double dx) {
    h.Draw("Col Z");
    gPad->Modified();
    gPad->Update();
    auto pt = dynamic_cast<TBox*>(h.GetListOfFunctions()->FindObject("palette"));
    if(!pt) return;
    pt->SetX2(pt->GetX2() + dx);
    pt->SetX1(pt->GetX1() + dx);
}

void setupSlideStyle(TStyle* S) {
    assert(S);
    S->SetOptStat("");
    S->SetLabelSize(0.05, "XYZ");
    //S->SetLabelOffset(0.02, "X");
    S->SetPadBottomMargin(0.14);
    S->SetPadLeftMargin(0.15);

    S->SetTitleSize(0.05,"xyz");
    S->SetTitleOffset(1.25,"z");
    S->SetTitleOffset(1.2,"y");
    S->SetTitleOffset(0.95,"x");
    S->SetTitleBorderSize(0);

    S->SetTitleW(1.0);
    S->SetTitleY(0.992);
    S->SetFillColor(0);
    S->SetHistLineWidth(2);
    S->SetLineWidth(2);
    S->SetNdivisions(507);
}
