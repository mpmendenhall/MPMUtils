/// \file GraphUtils.hh ROOT graph/histogram manipulation utilities
#ifndef GRAPHUTILS_HH
#define GRAPHUTILS_HH

#include "Stringmap.hh"
#include "AxisEnum.hh"
#include <TF1.h>
#include <TH1.h>
#include <TH1D.h>
#include <TH2F.h>
#include <TH3.h>
#include <TGraph.h>
#include <TProfile.h>
#include <TGraphErrors.h>
#include <TCanvas.h>
#include <vector>

/// bin edges for log histograms
vector<double> logbinedges(unsigned int nbins, double bmin, double bmax);
/// logarithmically binned histogram
template<class TH1x = TH1F>
TH1x* logHist(const string& name, const string& descrip, unsigned int nbins, double bmin, double bmax) {
    return new TH1x(name.c_str(), descrip.c_str(), nbins, logbinedges(nbins,bmin,bmax).data());
}
/// log-lin binned 2D histogram
template<class TH2x = TH2F>
TH2x* loglinHist(const string& name, const string& descrip, unsigned int nbins, double bmin, double bmax, unsigned int nby, double ymin, double ymax) {
    return new TH2x(name.c_str(), descrip.c_str(), nbins, logbinedges(nbins,bmin,bmax).data(), nby, ymin, ymax);
}
/// log-log binned 2D histogram
template<class TH2x = TH2F>
TH2x* loglogHist(const string& name, const string& descrip, unsigned int nbins, double bmin, double bmax, unsigned int nby, double ymin, double ymax) {
    return new TH2x(name.c_str(), descrip.c_str(), nbins, logbinedges(nbins,bmin,bmax).data(), nby, logbinedges(nby,ymin,ymax).data());
}
/// bin-to-bin derivative of histogram, with optional scale factor
TGraphErrors* histoDeriv(const TH1* h, unsigned int dxi = 1, double s = 1.0);

/// Fill histogram preserving *average* value interpolated between bins
void fill_interp(TH1* h, double x, double w = 1.0);

/// convert TH1* to Stringmap
Stringmap histoToStringmap(const TH1* h);
/// convert Stringmap to TH1F*
TH1F* stringmapToTH1F(const Stringmap& m);
/// convert a TGraph to a Stringmap
Stringmap graphToStringmap(const TGraph& g);

/// convert a histogram to a TGraph, optionally swapping x/y
TGraphErrors* TH1toTGraph(const TH1& h, bool invert = false);

/// convert a TProfile to a TGraph
TGraphErrors* TProf2TGraph(const TProfile& P, unsigned int minpts = 0);

/// make cumulative histogram
TH1* cumulativeHist(const TH1& h, bool normalize = false, bool reverse = false);

/// Divide out histogram bin width, for differential spectrum (with optional extra scale factor)
void normalize_to_bin_width(TH1* f, double xscale = 1.);
/// Divide out 2D histogram bin area (with optional extra scale factor)
void normalize_to_bin_area(TH2* h, double xscale = 1.);

/// Scale histogram by geometric bin center (for Lethargy plot)
void scale_times_bin_center(TH1* f);

/// invert a TGraph
TGraph* invertGraph(const TGraph* g);

/// combine a list of TGraphs
TGraph* combine_graphs(const vector<TGraph*> gs);

/// merge a list of TGrapherrors into a larger TGraphError, offsetting x values by given offsets
TGraphErrors* merge_plots(const vector<TGraphErrors*>& pin, const vector<int>& toffset);

/// draw several graphs on the same canvas
void drawTogether(vector<TGraphErrors*>& gs, float ymin, float ymax, TCanvas* C, const char* outname, const char* graphTitle);

/// find transform curve to match two histograms
TGraph* matchHistoShapes(const TH1F& h1, const TH1F& h2);

/// scale a TGraphErrors, on y (default) or x axis
void scale(TGraphErrors& tg, float s, bool xaxis=false);

/// shift all TGraph points
void shift(TGraph& g, double dx, double dy);

/// Add projection to all rows (columns) of TH2
void addProjection(TH2& h, const TH1& hP, double s = 1., bool xaxis = true);

/// accumulate TGraphErrors
void accumPoints(TGraphErrors& a, const TGraphErrors& b, bool errorWeight = true, bool yonly = false);

/// generate derivative graph
TGraph* derivative(TGraph& g);

/// Poisson-smear a histogram (with optional limiting resolution), preserving total counts
TH1* poisson_smear(const TH1& hIn, double NperX, TH1* hOut = nullptr, double n_max = 0);

/// transform axis on a TGraph, optionally using jacobean to preserve integral
void transformAxis(TGraph& g, TGraph& T, bool useJacobean = true);

/// interpolate a TGraphErrors to a new grid spacing ~dx
TGraphErrors* interpolate(TGraphErrors& tg, float dx);

/// get inverse CDF x: CDF(x)=p for histogram
double invCDF(TH1* h, double p);

/// check histogram for NaNs
void fixNaNs(TH1* h);

/// histogram integral and error specified by x axis range.
double integralAndError(TH1* h, double x0, double x1, Double_t& err, const string& option = "");

/// histogram integral and error with fractional-bin-interpolated result; optional to multiply by dx.
double integralAndErrorInterp(TH1* h, double x0, double x1, Double_t& err, bool dxmul = true);

/// create 1D histogram binned with axis from higher-dimensional histogram
TH1F* axisHist(const TH2& h, const string& hname, AxisDirection d);

/// slice a TH3 into a stack of TH2Fs
vector<TH2F*> sliceTH3(const TH3& h3, AxisDirection d = Z_DIRECTION);

/// slice a TH2 into a stack of TH1Fs
vector<TH1F*> sliceTH2(const TH2& h2, AxisDirection d, bool includeOverflow = false);

/// split a list of counts into approximately equal segments
vector<unsigned int> equipartition(const vector<float>& elems, unsigned int n);

/// project a TH2 onto an arbitrary line
TH1* projectTH2(const TH2& h, double nb, double cx, double cy);

/// get plot indicating optimal separation point between two histograms
TH1* histsep(const TH1& h1, const TH1& h2);

/// calculate optimum dividing point and overlap error between two histograms
void histoverlap(const TH1& h1, const TH1& h2, double& xdiv, double& o);

#endif
