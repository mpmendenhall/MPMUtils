/// \file GraphicsUtils.hh ROOT graphics utilities
#ifndef GRAPHICSUTILS_HH
#define GRAPHICSUTILS_HH

#include "AxisEnum.hh"
#include <math.h>
#include <TVirtualPad.h>
#include <TPolyLine.h>
#include <TStyle.h>
#include <TLine.h>
#include <TEllipse.h>
#include <TH1.h>
#include <TH2.h>

#include <vector>
#include <TH1.h>

using std::vector;
using std::string;

/// draw several histograms simultaneously; return max histogram height
double drawSimulHistos(vector<TH1*>& hists, const string& opt = "", const string& newTitle = "DEFAULT");
/// draw a pair of histograms (red and blue by default)
void drawHistoPair(TH1* hRed, TH1* hBlue, const string& opt = "", Int_t c1 = 2, Int_t c2 = 4);
/// draw a pair of histograms, with line and diamond markers
void drawDataMCPair(TH1* dat, TH1* mc);

/// Draw list of objects into multi-page PDF
void combo_draw_objs(const vector<TObject*>& hs, const string& outpath, const char* opt = "");
/// Draw list of objects into multi-page PDF
template<class T = TObject>
void combo_draw(const vector<T*>& hs, const string& outpath, const char* opt = "") {
    vector<TObject*> v(hs.begin(), hs.end());
    combo_draw_objs(v, outpath, opt);
}

/// draw vertical line marker
TLine* drawVLine(Float_t x, TVirtualPad* C, Int_t color = 4, Int_t style = 1);
/// draw horizontal line
TLine* drawHLine(Float_t y, TVirtualPad* C, Int_t color = 4, Int_t style = 1);
/// draw circle
TEllipse* drawCircle(float r, Int_t color = 1, Int_t lstyle = 1, float x0=0, float y0=0);
/// draw shaded rectangle marker
void drawExcludedRegion(Float_t x0, Float_t x1, TCanvas* C, Int_t color = 4, Int_t fill = 1001);

/// make ellipse from center and inverse covariance matrix arrays (for Matrix/Clustering.hh)
TPolyLine* makeEllipse(float x0, float y0, const double* iSigma);

/// set up grayscale figures color palette, black-to-white by default, or inverted
void makeGrayscalepalette(bool b2w = true);
/// set up blue-white-red palette
void makeRBpalette();

/// gStyle settings for "slideshow-ready" plots
void setupSlideStyle(TStyle* S = gStyle);

/// filter out empty histograms
template<class C>
vector<C*> nonEmptyHistos(const vector<C*>& v) {
    vector<C*> vv;
    for(auto h: v) if(h->GetEntries()) vv.push_back(h);
    return vv;
}


#endif
