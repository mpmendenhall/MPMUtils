/// \file SplineFit.cc
#include "SplineFit.hh"
#include "StringManip.hh"

int SplineFit::nameCounter = 0;

void SplineFit::setX(const double* x, size_t n) {
    if(myFitter && myFitter->GetNpar() != int(n)) {
        delete myFitter;
        myFitter = NULL;
    }
    vector<double> y(n);
    mySpline = TGraphErrors(n,x,y.data());
}

TF1* SplineFit::getFitter() {
    if(!myFitter) {
        size_t n = mySpline.GetN();
        myFitter = new TF1(("splineFit_"+to_str(nameCounter++)).c_str(), this, &SplineFit::eval, mySpline.GetX()[0], mySpline.GetX()[n-1], n);
    }
    return myFitter;
}

TF1* SplineFit::getFitter(const TH1* h) {
    getFitter();
    const size_t n = mySpline.GetN();
    for(size_t i=0; i<n; i++) myFitter->SetParameter(i,h->GetBinContent(h->GetXaxis()->FindBin(mySpline.GetX()[i])));
    return myFitter;
}

TF1* SplineFit::getFitter(const TGraph* g) {
    getFitter();
    const size_t n = mySpline.GetN();
    for(size_t i=0; i<n; i++) myFitter->SetParameter(i,g->Eval(mySpline.GetX()[i]));
    return myFitter;
}

void SplineFit::updateSpline() {
    if(!myFitter) return;
    for(int i=0; i<mySpline.GetN(); i++) {
        mySpline.GetY()[i] = myFitter->GetParameter(i);
        mySpline.GetEY()[i] = myFitter->GetParError(i);
    }
}
    
double SplineFit::eval(double* x, double* p) {
    const size_t n = mySpline.GetN();
    for(size_t i=0; i<n; i++) mySpline.GetY()[i] = p[i];
    return mySpline.Eval(*x);
}
