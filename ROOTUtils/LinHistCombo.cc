/// @file LinHistCombo.cc Least-squares fitter for linear combinations of histograms
/*
 * LinHistCombo.cc, part of the MPMUtils package.
 * Copyright (c) 2007-2014 Michael P. Mendenhall
 *
 * This code uses the LAPACKE C interface to LAPACK;
 * see http://www.netlib.org/lapack/lapacke.html
 * and the GSL interface to CBLAS, https://www.gnu.org/software/gsl/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "LinHistCombo.hh"
#include "StringManip.hh"

unsigned int LinHistCombo::nFitters = 0;

TF1* LinHistCombo::getFitter() {
    if(myFit && myFit->GetNpar() != (int)terms.size()) { delete myFit; myFit = nullptr; }
    if(!myFit) myFit = new TF1(("fCombo"+to_str(nFitters++)).c_str(), this, &LinHistCombo::Evaluate,0,1,terms.size());
    return myFit;
}

int LinHistCombo::Fit(TH1* h, double xmin, double xmax, const std::string& fitopt) {
    getFitter();
    myFit->SetRange(xmin,xmax);
    int err = h->Fit(myFit,fitopt.c_str());
    coeffs.clear();
    dcoeffs.clear();
    for(unsigned int i=0; i<terms.size(); i++) {
        coeffs.push_back(myFit->GetParameter(i));
        dcoeffs.push_back(myFit->GetParError(i));
    }
    return err;
}

double interplhist(TH1* h, double x) {
    int b0 = h->FindBin(x);
    if(b0<1 || b0 > h->GetNbinsX()) return 0;
    double w = h->GetBinWidth(b0);
    double l = (x-h->GetBinCenter(b0))/w;
    if(l<=0) {
        l += 1.0;
        b0 -= 1;
    }
    double y1 = b0<1?0:h->GetBinContent(b0);
    double y2 = b0+1<=h->GetNbinsX()?h->GetBinContent(b0+1):0;
    return y1*(1-l)+y2*l;
}

double LinHistCombo::Evaluate(double* x, double* p) {
    double s = 0;
    for(unsigned int i=0; i<terms.size(); i++) {
        if(interpolate) {
            s += p[i]*interplhist(terms[i],*x);
        } else {
            int bn = terms[i]->FindBin(*x);
            if(bn < 1 || bn >= terms[i]->GetNbinsX()-1) continue;
            s += terms[i]->GetBinContent(bn)*p[i];
        }
    }
    return s;
}

void LinHistCombo::forceNonNegative() {
    getFitter();
    for(unsigned int i=0; i<terms.size(); i++)
        myFit->SetParLimits(i,0,100);
}
