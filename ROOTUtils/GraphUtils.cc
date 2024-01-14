/// @file GraphUtils.cc
#include "GraphUtils.hh"
#include "to_str.hh"
#include <math.h>
#include <TROOT.h>
#include <TMath.h>
#include <TDirectory.h>
#include <cmath>
#include <cassert>

vector<double> logbinedges(unsigned int nbins, double bmin, double bmax) {
    if(!(bmin > 0 && bmax > 0)) throw std::logic_error("Zero or negative logarithmic axis limits requested");
    vector<Double_t> binEdges(nbins+1);
    for(unsigned int i=0; i<=nbins; i++)
        binEdges[i] = exp((nbins-i)*log(bmin)/nbins + i*log(bmax)/nbins);
    return binEdges;
}

void fill_interp(TH1& h, double x, double w) {
    auto Ax = h.GetXaxis();
    int b0 = Ax->FindBin(x);
    if(b0 < 1 || b0 > h.GetNbinsX()) { h.Fill(x); return; }
    double c0 = Ax->GetBinCenter(b0);
    int b1 = x > c0? b0+1 : b0-1;
    double c1 = Ax->GetBinCenter(b1);
    double a = (c1-x)/(c1-c0);
    h.Fill(c0, a*w);
    h.Fill(c1, (1-a)*w);
}

void normalize_to_bin_width(TH1* f, double xscale, const string& ytitle) {
    if(!f) return;
    Int_t bx,by,bz;
    for(int i=0; i<f->GetNcells(); ++i) {
        f->GetBinXYZ(i,bx,by,bz);
        TAxis* A = f->GetXaxis();
        double s = 1./A->GetBinWidth(bx);
        f->SetBinContent(i, f->GetBinContent(i)*s);
        f->SetBinError(i, f->GetBinError(i)*s);
    }
    f->Scale(xscale);
    if(ytitle.size()) f->GetYaxis()->SetTitle(ytitle.c_str());
}

void addConst(TH1& h, double c) {
    for(int i=0; i<h.GetNcells(); ++i) h.SetBinContent(i, h.GetBinContent(i) + c);
}

void normalize_to_bin_area(TH2* h, double xscale) {
    if(!h) return;
    TAxis* Ax = h->GetXaxis();
    TAxis* Ay = h->GetYaxis();
    Int_t bx,by,bz;
    for(int i=0; i<h->GetNcells(); i++) {
        h->GetBinXYZ(i,bx,by,bz);
        if(!bx || !by || bx > Ax->GetNbins() || by > Ay->GetNbins()) continue;
        double s = 1./Ax->GetBinWidth(bx)/Ay->GetBinWidth(by);
        h->SetBinContent(i, h->GetBinContent(i)*s);
        h->SetBinError(i, h->GetBinError(i)*s);
    }
    h->Scale(xscale);
}

void addProjection(TH2& h, const TH1& hP, double s, bool xaxis) {
    if(xaxis) {
        assert(h.GetNbinsX() == hP.GetNbinsX());
        for(Int_t nx=0; nx<=h.GetNbinsX()+1; nx++) {
            double dz = hP.GetBinContent(nx)*s;
            for(Int_t ny=1; ny<=h.GetNbinsY(); ny++) {
                Int_t b = h.GetBin(nx,ny);
                h.SetBinContent(b, h.GetBinContent(b) + dz);
            }
        }
    } else {
        assert(h.GetNbinsY() == hP.GetNbinsX());
        for(Int_t nx=0; nx<=h.GetNbinsY()+1; nx++) {
            double dz = hP.GetBinContent(nx)*s;
            for(Int_t ny=1; ny<=h.GetNbinsX(); ny++) {
                Int_t b = h.GetBin(ny,nx);
                h.SetBinContent(b, h.GetBinContent(b) + dz);
            }
        }
    }
}

void scale_times_bin_center(TH1* f) {
    if(!f) return;
    Int_t bx,by,bz;
    for(int i=0; i<f->GetNcells(); i++) {
        f->GetBinXYZ(i,bx,by,bz);
        TAxis* A = f->GetXaxis();
        double s = sqrt(A->GetBinLowEdge(bx)*A->GetBinUpEdge(bx));
        f->SetBinContent(i, f->GetBinContent(i)*s);
        f->SetBinError(i, f->GetBinError(i)*s);
    }
    f->Scale(1.0);
}

TGraphErrors* histoDeriv(const TH1& h, unsigned int dxi, double s) {
    int nb = h.GetNbinsX();
    TGraphErrors* g = new TGraphErrors();
    int n = 0;
    for(int i=1+dxi/2; i<=nb; i+=dxi) {
        double x0 = h.GetBinCenter(i);
        double x1 = h.GetBinCenter(i+dxi);
        double dx = x1-x0;
        double dy = h.GetBinContent(i+dxi) - h.GetBinContent(i);
        g->SetPoint(n, 0.5*(x0+x1), s*dy/dx);
        g->SetPointError(n, 0, sqrt(pow(h.GetBinError(i),2)+pow(h.GetBinError(i+dxi),2))*s/dx);
        n++;
    }
    return g;
}

TGraphErrors* TH1toTGraph(const TH1& h, bool invert) {
    TGraphErrors* g = new TGraphErrors(h.GetNbinsX());
    for(int i=0; i<h.GetNbinsX(); i++) {
        if(invert) {
            g->SetPoint(i, h.GetBinContent(i+1), h.GetBinCenter(i+1));
            g->SetPointError(i, h.GetBinError(i+1), 0.);
        } else {
            g->SetPoint(i, h.GetBinCenter(i+1), h.GetBinContent(i+1));
            g->SetPointError(i, 0, h.GetBinError(i+1));
        }
    }
    return g;
}

TGraphErrors* TProf2TGraph(const TProfile& P, unsigned int minpts) {
    TGraphErrors* g = new TGraphErrors(P.GetNbinsX());
    unsigned int ig = 0;
    for(int i=0; i<P.GetNbinsX(); i++) {
        if(P.GetBinEntries(i+1)<minpts) continue;
        g->SetPoint(ig,P.GetBinCenter(i+1),P.GetBinContent(i+1));
        g->SetPointError(ig,0.0,P.GetBinError(i+1));
        ig++;
    }
    while(g->GetN()>(int)ig) g->RemovePoint(ig);
    return g;
}

void comboErr(double a, double da, double b, double db, double& x, double& dx) {
    da *= da;
    db *= db;
    if(!da) da = 1e-16;
    if(!db) db = 1e-16;
    double nrm = 1./(1./da+1./db);
    x = (a/da+b/db)*nrm;
    dx = sqrt(nrm);
}

void accumPoints(TGraphErrors& a, const TGraphErrors& b, bool errorWeight, bool yonly) {
    assert(a.GetN() == b.GetN());
    for(int i=0; i<a.GetN(); i++) {
        double ax,ay,bx,by;
        a.GetPoint(i,ax,ay);
        b.GetPoint(i,bx,by);
        double dax = a.GetErrorX(i);
        double day = a.GetErrorY(i);
        double dbx = b.GetErrorX(i);
        double dby = b.GetErrorY(i);
        if(errorWeight) {
            double x,dx,y,dy;
            comboErr(ax, dax, bx, dbx, x, dx);
            comboErr(ay, day, by, dby, y, dy);
            if(!yonly) a.SetPoint(i,x,y);
            a.SetPointError(i,dx,dy);
        } else {
            if(yonly) {
                a.SetPoint(i,ax,ay+by);
                a.SetPointError(i,dax,sqrt(day*day+dby*dby));
            } else {
                a.SetPoint(i,ax+bx,ay+by);
                a.SetPointError(i,sqrt(dax*dax+dbx*dbx),sqrt(day*day+dby*dby));
            }
        }
    }
}

TH1* cumulativeHist(const TH1& h, bool normalize, bool reverse) {
    auto c = static_cast<TH1*>(h.Clone((h.GetName()+string("_cum")).c_str()));
    int n = h.GetNbinsX();
    float ecum2 = 0;
    if(reverse) {
        //c->SetBinContent(n+1,0);
        //c->SetBinError(n+1,0);
        for(int i=n; i>=0; i--) {
            c->SetBinContent(i,c->GetBinContent(i+1)+h.GetBinContent(i));
            ecum2 += pow(h.GetBinError(i),2);
            c->SetBinError(i,sqrt(ecum2));
        }
    } else {
        //c->SetBinContent(0,0);
        //c->SetBinError(0,0);
        for(int i=1; i<=n+1; i++) {
            c->SetBinContent(i,c->GetBinContent(i-1)+h.GetBinContent(i));
            ecum2 += pow(h.GetBinError(i),2);
            c->SetBinError(i,sqrt(ecum2));
        }
    }

    if(normalize) c->Scale(1.0/c->GetBinContent(reverse?1:n));
    return c;
}

TGraph invertGraph(const TGraph& g) {
    TGraph gi(g.GetN());
    double x,y;
    for(int i=0; i<g.GetN(); i++) {
        g.GetPoint(i,x,y);
        gi.SetPoint(i,y,x);
    }
    return gi;
}

TGraph sumGraphs(const TGraph& g0, const TGraph& g1) {
    auto n0 = g0.GetN();
    if(!n0) return g1;
    auto n1 = g1.GetN();
    if(!n1) return g0;

    assertSorted(g0);
    assertSorted(g1);

    auto X0 = g0.GetX();
    auto X1 = g1.GetX();
    auto Y0 = g0.GetY();
    auto Y1 = g1.GetY();

    TGraph g;

    // initial non-overlapping range, including last point if exact overlap at ends
    int i=0, i0=0, i1=0;
    /**/ if(X0[0] < X1[0]) while(i0 < n0 && X0[i0] <= X1[0]) { g.SetPoint(++i, X0[i0], Y0[i0]); ++i0; }
    else if(X1[0] < X0[0]) while(i1 < n1 && X1[i1] <= X0[0]) { g.SetPoint(++i, X1[i1], Y1[i1]); ++i1; }

    // overlapping range
    while(i0 < n0 && i1 < n1) {
        if(X0[i0] == X1[i1])      { g.SetPoint(++i, X0[i0], Y0[i0] + Y1[i1]);   ++i0;  ++i1; }
        else if(X0[i0] < X1[i1])  { g.SetPoint(++i, X0[i0], Y0[i0] + g1.Eval(X0[i0])); ++i0; }
        else                      { g.SetPoint(++i, X1[i1], Y1[i1] + g0.Eval(X1[i1])); ++i1; }
    }

    // final non-overlapping range
    while(i0 < n0) { g.SetPoint(++i, X0[i0], Y0[i0]); ++i0; }
    while(i1 < n1) { g.SetPoint(++i, X1[i1], Y1[i1]); ++i1; }

    g.SetBit(TGraph::kIsSortedX);
    return g;
}

TGraphErrors* merge_plots(const vector<TGraphErrors*>& pin, const vector<int>& toffset) {
    printf("Merging %i graphs...\n",(int)pin.size());
    unsigned int npts = 0;
    for(unsigned int n=0; n<pin.size(); n++)
        npts += pin[n]->GetN();
    TGraphErrors* tg = new TGraphErrors(npts);
    npts = 0;
    double x,y;
    for(unsigned int n=0; n<pin.size(); n++) {
        for(int n2 = 0; n2 < pin[n]->GetN(); n2++) {
            pin[n]->GetPoint(n2,x,y);
            tg->SetPoint(npts,(x+toffset[n])/3600.0,y);
            tg->SetPointError(npts,pin[n]->GetErrorX(n2)/3600.0,pin[n]->GetErrorY(n2));
            ++npts;
        }
    }
    tg->GetXaxis()->SetTitle("Time [Hours]");
    return tg;
}

void drawTogether(vector<TGraphErrors*>& gs, float ymin, float ymax, TCanvas* C, const char* outname, const char* graphTitle) {
    if(!gs.size())
        return;
    for(unsigned int t=0; t<gs.size(); t++)
        gs[t]->SetLineColor(t+1);
    gs[0]->SetMinimum(ymin);
    gs[0]->SetMaximum(ymax);
    gs[0]->SetTitle(graphTitle);
    gs[0]->Draw("AP");
    for(unsigned int i=1; i<gs.size(); i++)
        gs[i]->Draw("P");
    C->Print(outname);

}



double integralAndErrorInterp(TH1* h, double x0, double x1, Double_t& err, bool dxmul) {
    err = 0;
    if(!h) return 0;
    const TAxis* Ax = h->GetXaxis();
    int b0 = Ax->FindBin(x0);
    int b1 = Ax->FindBin(x1);
    double ss = 0;
    if(b0+1 <= b1-1) ss += h->IntegralAndError(b0+1, b1-1, err, dxmul?"width":"");
    double fx0 = (Ax->GetBinUpEdge(b0) - x0)*(dxmul? 1 : 1./Ax->GetBinWidth(b0));
    double fx1 = (x1 - Ax->GetBinLowEdge(b1))*(dxmul? 1 : 1./Ax->GetBinWidth(b1));
    ss += fx0*h->GetBinContent(b0);
    ss += fx1*h->GetBinContent(b1);
    double e0 = fx0*h->GetBinError(b0);
    double e1 = fx1*h->GetBinError(b1);
    err = sqrt(err*err + e0*e0 + e1*e1);
    return ss;
}

double integralAndError(TH1* h, double x0, double x1, Double_t& err, const string& option) {
    err = 0;
    if(!h) return 0;
    const TAxis* Ax = h->GetXaxis();
    int b0 = Ax->FindBin(x0);
    int b1 = Ax->FindBin(x1);
    return h->IntegralAndError(b0, b1, err, option.c_str());
}

TH1* poisson_smear(const TH1& hIn, double NperX, TH1* hOut, double n_max) {
    if(!hOut) {
        hOut = static_cast<TH1*>(hIn.Clone((string(hIn.GetName())+"_Smeared").c_str()));
        hOut->Reset();
    }
    for(int i=1; i<=hIn.GetNbinsX(); i++) {
        double c0 = hIn.GetBinContent(i);
        if(!c0) continue;
        double X = hIn.GetBinCenter(i);
        double n0 = X*NperX;
        if(n_max && n0) n0 = 1./(1./n0 + 1./n_max);
        double NperXeff = n0/X;
        double nrm = 0;
        for(int j=1; j<=hOut->GetNbinsX(); j++) nrm += TMath::Poisson(hOut->GetBinCenter(j)*NperXeff, n0);
        for(int j=1; j<=hOut->GetNbinsX(); j++) {
            double X1 = hOut->GetBinCenter(j);
            hOut->Fill(X1, c0 * TMath::Poisson(X1*NperXeff, n0)/nrm);
        }
    }
    return hOut;
}

TGraph* matchHistoShapes(const TH1F& h1, const TH1F& h2) {
    auto c1 = cumulativeHist(h1,true);
    auto c2 = cumulativeHist(h2,true);

    auto c2g = TH1toTGraph(*c2);
    delete c2;

    auto c2i = invertGraph(*c2g);
    delete c2g;

    int n = h1.GetNbinsX()-2;
    TGraph* T = new TGraph(n);
    for(int i=1; i<=n; i++)
        T->SetPoint(i-1,c1->GetBinCenter(i),c2i.Eval(c1->GetBinContent(i)));
    delete c1;
    return T;
}

void scale(TGraphErrors& tg, float s, bool xaxis) {
    double x,y;
    for(int i=0; i<tg.GetN(); i++) {
        tg.GetPoint(i,x,y);
        if(xaxis) {
            tg.SetPoint(i, x*s, y);
            tg.SetPointError(i, tg.GetErrorX(i)*s, tg.GetErrorY(i));
    } else {
            tg.SetPoint(i, x, s*y);
            tg.SetPointError(i, tg.GetErrorX(i), s*tg.GetErrorY(i));
        }
    }
}

void shift(TGraph& g, double dx, double dy) {
    double x,y;
    for(int i=0; i<g.GetN(); i++) {
        g.GetPoint(i,x,y);
        g.SetPoint(i, x+dx, y+dy);
    }
}

TGraph* derivative(TGraph& g) {
    g.Sort();
    TGraph* d = new TGraph(g.GetN()-1);
    double x1,y1,x2,y2;
    g.GetPoint(0,x1,y1);
    for(int i=0; i<g.GetN()-1; i++) {
        g.GetPoint(i+1,x2,y2);
        d->SetPoint(i,0.5*(x1+x2),(y2-y1)/(x2-x1));
        x1 = x2;
        y1 = y2;
    }
    return d;
}

void transformAxis(TGraph& g, TGraph& T, bool useJacobean) {
    double x,y,j;
    j = 1.0;
    TGraph* d = useJacobean? derivative(T) : nullptr;
    for(int i=0; i<g.GetN(); i++) {
        g.GetPoint(i,x,y);
        if(d) j = d->Eval(x);
        g.SetPoint(i,T.Eval(x),j*y);
    }
    delete d;
}

TGraphErrors* interpolate(TGraphErrors& tg, float dx) {
    vector<float> xnew;
    vector<float> ynew;
    vector<float> dynew;

    // sort input points by x value
    tg.Sort();

    // interpolate each interval of the original graph
    for(int i=0; i<tg.GetN()-1; i++) {
        double x0,x1,y;
        tg.GetPoint(i,x0,y);
        tg.GetPoint(i+1,x1,y);
        double dy0 = tg.GetErrorY(i);
        double dy1 = tg.GetErrorY(i+1);
        // determine number of points for this interval
        int ninterp = (x1-x0>dx)?int((x1-x0)/dx):1;
        for(int n=0; n<ninterp; n++) {
            float l = float(n)/float(ninterp);
            xnew.push_back(x0+(x1-x0)*l);
            ynew.push_back(tg.Eval(xnew.back()));
            dynew.push_back(sqrt(ninterp)*((1-l)*dy0+l*dy1));
        }
    }

    // fill interpolated output graph
    TGraphErrors* gout = new TGraphErrors(xnew.size());
    for(unsigned int i=0; i<xnew.size(); i++) {
        gout->SetPoint(i,xnew[i],ynew[i]);
        gout->SetPointError(i,0,dynew[i]);
    }
    return gout;
}

double invCDF(TH1& h, double p) {
    unsigned int nbins = h.GetNbinsX()-2;
    if(p <= 0.) return 0;
    if(p >= 1.) return h.GetBinLowEdge(nbins+1);
    Double_t* cdf = h.GetIntegral();
    unsigned int mybin = std::upper_bound(cdf,cdf+nbins,p)-cdf;
    assert(mybin>0);
    assert(mybin<=nbins);
    double l = (p-cdf[mybin-1])/(cdf[mybin]-cdf[mybin-1]);
    return h.GetBinLowEdge(mybin)*(1.0-l) + h.GetBinLowEdge(mybin+1)*l;
}

double hcount_from_end(const TH1& h, double c) {
    double s = 0;
    int i = h.GetNbinsX();
    for(; i>1; --i) {
        double bc = h.GetBinContent(i);
        s += bc;
        if(s >= c) {
            auto fb = (s-c)/bc;
            return h.GetBinLowEdge(i)*fb + (1-fb)*h.GetBinLowEdge(i+1);
        }
    }
    return h.GetBinLowEdge(1);
}

void fixNaNs(TH1* h) {
    unsigned int nb = h->GetNbinsX();
    for(unsigned int i=0; i<=nb+1; i++) {
        if(!(h->GetBinContent(i) == h->GetBinContent(i))) {
            printf("NaN found in bin %u/%u!\n",i,nb);
            h->SetBinContent(i,0);
            h->SetBinError(i,0);
        }
    }
}

TH1F* axisHist(const TH2& h, const string& hname,  AxisDirection_t d) {
    const TAxis* Ax = (d==X_DIRECTION? h.GetXaxis() : d ==Y_DIRECTION?  h.GetYaxis() : h.GetZaxis());
    int nbins = Ax->GetNbins();
    TH1F* h1;
    if(Ax->IsVariableBinSize()) {
        vector<double> binedges;
        for(int i=1; i<=nbins; i++) binedges.push_back(Ax->GetBinLowEdge(i));
        binedges.push_back(Ax->GetBinUpEdge(nbins));
        h1 = new TH1F(hname.c_str(), h.GetTitle(), nbins, binedges.data());
    } else {
        h1 = new TH1F(hname.c_str(), h.GetTitle(), nbins, Ax->GetBinLowEdge(1), Ax->GetBinUpEdge(nbins));
    }
    if(h.GetSumw2()) h1->Sumw2();
    h1->GetXaxis()->SetTitle(Ax->GetTitle());
    return h1;
}

vector<TH2F*> sliceTH3(const TH3& h3, AxisDirection_t d) {
    const TAxis* Ax1 = d==X_DIRECTION? h3.GetYaxis() : h3.GetXaxis();
    const TAxis* Ax2 = d==Z_DIRECTION? h3.GetYaxis() : h3.GetZaxis();
    const TAxis* Ax3 = d==X_DIRECTION? h3.GetXaxis() : d==Y_DIRECTION? h3.GetYaxis() : h3.GetZaxis();
    const unsigned int n1 = Ax1->GetNbins();
    const unsigned int n2 = Ax2->GetNbins();
    const unsigned int n3 = Ax3->GetNbins();

    vector<TH2F*> h2s;
    for(unsigned int z = 0; z <= n3+1; z++) {
        TH2F* h2 = new TH2F((std::string(h3.GetName())+"_"+to_str(z)).c_str(),
        h3.GetTitle(),
        n1,
        Ax1->GetBinLowEdge(1),
        Ax1->GetBinUpEdge(n1),
        n2,
        Ax2->GetBinLowEdge(1),
        Ax2->GetBinUpEdge(n2));
        if(h3.GetSumw2()) h2->Sumw2();
        h2->GetXaxis()->SetTitle(Ax1->GetTitle());
        h2->GetYaxis()->SetTitle(Ax2->GetTitle());
        for(unsigned int x=0; x <= n1+1; x++) {
            for(unsigned int y=0; y <= n2+1; y++) {
                if(d==X_DIRECTION) {
                    h2->SetBinContent(x,y,h3.GetBinContent(z,x,y));
                    h2->SetBinError(x,y,h3.GetBinError(z,x,y));
                } else if(d==Y_DIRECTION) {
                    h2->SetBinContent(x,y,h3.GetBinContent(x,z,y));
                    h2->SetBinError(x,y,h3.GetBinError(x,z,y));
                } else if(d==Z_DIRECTION) {
                    h2->SetBinContent(x,y,h3.GetBinContent(x,y,z));
                    h2->SetBinError(x,y,h3.GetBinError(x,y,z));
                }
            }
        }
        h2s.push_back(h2);
    }
    return h2s;
}

vector<TH1F*> sliceTH2(const TH2& h2, AxisDirection_t d, bool includeOverflow) {
    vector<TH1F*> h1s;
    const unsigned int nx = h2.GetNbinsX();
    const unsigned int ny = h2.GetNbinsY();
    const unsigned int nz = d==X_DIRECTION? nx : ny;
    const unsigned int nn = d==X_DIRECTION? ny : nx;

    for(unsigned int z = 0; z <= nz+1; z++) {
        if(!includeOverflow && (z == 0 || z == nz+1)) continue;
        TH1F* h1 = axisHist(h2, h2.GetName()+("_"+to_str(z)), d==X_DIRECTION? Y_DIRECTION : X_DIRECTION);
        h1->GetYaxis()->SetTitle(h2.GetZaxis()->GetTitle());
        for(unsigned int n=0; n <= nn+1; n++) {
            if(d==X_DIRECTION) {
                h1->SetBinContent(n,h2.GetBinContent(z,n));
                h1->SetBinError(n,h2.GetBinError(z,n));
            } else {
                h1->SetBinContent(n,h2.GetBinContent(n,z));
                h1->SetBinError(n,h2.GetBinError(n,z));
            }
        }
        h1s.push_back(h1);
    }
    return h1s;
}

vector<unsigned int> equipartition(const vector<float>& elems, unsigned int n) {
    vector<float> cumlist;
    for(unsigned int i=0; i<elems.size(); i++)
        cumlist.push_back(i?cumlist[i-1]+elems[i]:elems[i]);

    vector<unsigned int> part;
    part.push_back(0);
    for(unsigned int i=1; i<n; i++) {
        double x0 = cumlist.back()*float(i)/float(n);
        unsigned int p = (unsigned int)(std::upper_bound(cumlist.begin(),cumlist.end(),x0)-cumlist.begin()-1);
        if(p != part.back()) part.push_back(p);
    }
    part.push_back(elems.size());
    return part;
}

TH1* projectTH2(const TH2& h, double nb, double cx, double cy) {
    const TAxis* Ax = h.GetXaxis();
    const TAxis* Ay = h.GetYaxis();
    double x0 = Ax->GetXmin();
    double x1 = Ax->GetXmax();
    double y0 = Ay->GetXmin();
    double y1 = Ay->GetXmax();
    TH1D* hOut = new TH1D((h.GetName()+std::string("_Projected")).c_str(),"Projected Histogram",nb,
    cx*(cx>0?x0:x1)+cy*(cy>0?y0:y1),cx*(cx>0?x1:x0)+cy*(cy>0?y1:y0));
    for(int bx=1; bx<=Ax->GetNbins(); bx++)
        for(int by=1; by<=Ay->GetNbins(); by++)
            hOut->Fill(cx*Ax->GetBinCenter(bx)+cy*Ay->GetBinCenter(by),h.GetBinContent(bx,by));
    return hOut;
}

TH1* histsep(const TH1& h1, const TH1& h2) {
    int nb = h1.GetNbinsX();
    assert(nb==h2.GetNbinsX());
    auto hDiv = static_cast<TH1*>(h1.Clone("hDivision"));
    hDiv->SetBinContent(0,0);
    hDiv->SetBinContent(nb+1,0);
    for(int b=1; b<=nb; b++)
        hDiv->SetBinContent(b,hDiv->GetBinContent(b-1)+h2.GetBinContent(b));
    double c = 0;
    for(int b=nb; b>=1; b--) {
        c += (b==nb?0:h1.GetBinContent(b+1));
        hDiv->SetBinContent(b,hDiv->GetBinContent(b)+c);
        hDiv->SetBinError(b,0);
    }
    return hDiv;
}

void histoverlap(const TH1& h1, const TH1& h2, double& xdiv, double& o) {
    int nb = h1.GetNbinsX();
    assert(nb==h2.GetNbinsX());
    double* csum = new double[nb+2];
    csum[0] = csum[nb+1] = 0;
    for(int b=1; b<=nb; b++)
        csum[b] = csum[b-1]+h2.GetBinContent(b);
    double c = 0;
    int bmn = nb;
    for(int b=nb; b>=1; b--) {
        c += (b==nb?0:h1.GetBinContent(b+1));
        csum[b] += c;
        if(csum[b] <= csum[bmn]) bmn=b;
    }
    o = csum[bmn];
    xdiv = h1.GetBinLowEdge(bmn+1);
    delete[] csum;
}

TGraph derivative_logspaced(const TGraph& g, double x0, double x1, double ysc, size_t npts) {
    TGraph dg(npts-1);
    vector<double> xs(npts);
    vector<double> ys(npts);
    for(size_t n=0; n<npts; n++) {
        double l = double(n)/(npts-1);
        xs[n] = exp((1-l)*log(x0) + l*log(x1));
        ys[n] = g.Eval(xs[n]);
        if(n > 0) {
            double xm = sqrt(xs[n]*xs[n-1]);
            double dydx = ysc*(ys[n]-ys[n-1])/(xs[n]-xs[n-1]);
            dg.SetPoint(n-1, xm, dydx);
        }
    }
    return dg;
}
