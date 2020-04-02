/// \file MultiGaus.cc
/*
 * MultiGaus.cc, part of the MPMUtils package.
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

#include "MultiGaus.hh"

MultiGaus::~MultiGaus() { delete myTF1; }

void MultiGaus::setParameter(unsigned int n, double p) {
    if(n%3==2) p = fabs(p);
    iguess.at(n) = p;
    myTF1->SetParameter(n,p);
}

double MultiGaus::getParameter(unsigned int n) const { return myTF1->GetParameter(n); }

double MultiGaus::getParError(unsigned int n) const { return myTF1->GetParError(n); }

float_err MultiGaus::getPar(unsigned int n) const {
    return float_err(getParameter(n),getParError(n));
}

void MultiGaus::fitEstimate(TH1* h, unsigned int n) {
    if(n>=npks) {
        for(unsigned int i=0; i<npks; i++) fitEstimate(h,i);
        return;
    }
    TF1 fGaus("fGausEst", "gaus", iguess[3*n+1]-iguess[3*n+2], iguess[3*n+1]+iguess[3*n+2]);
    h->Fit(&fGaus,"QNR");
    for(int i=0; i<3; i++) iguess[3*n+i] = fGaus.GetParameter(i);
}

void MultiGaus::setCenterSigma(unsigned int n, double c, double s) {
    if(n>=npks) return;
    setParameter(3*n+1, c);
    setParameter(3*n+2, s);
}

TF1* MultiGaus::getFitter() {
    double xmin,xmax;
    xmin = xmax = iguess[1];
    for(unsigned int p=0; p<npks; p++) {
        if(iguess[3*p+1]-nSigma*iguess[3*p+2] < xmin)
            xmin = iguess[3*p+1]-nSigma*iguess[3*p+2];
        if(iguess[3*p+1]+nSigma*iguess[3*p+2] > xmax)
            xmax = iguess[3*p+1]+nSigma*iguess[3*p+2];
        for(unsigned int i=0; i<3; i++)
            myTF1->SetParameter(3*p+i,iguess[3*p+i]);
    }
    myTF1->SetRange(xmin,xmax);
    return myTF1;
}

void MultiGaus::fit(TH1* h, bool draw) {
    for(unsigned int i=0; i<npks; i++)
        iguess[3*i+0] = h->GetBinContent(h->FindBin(iguess[3*i+1]));
    if(draw)
        h->Fit(getFitter(),"QR");
    else
        h->Fit(getFitter(),"QRN");

    for(unsigned int i=0; i<3*npks; i++) {
        if(i%3==2)
            iguess[i] = fabs(myTF1->GetParameter(i));
        else
            iguess[i] = myTF1->GetParameter(i);
    }
}

void MultiGaus::display() const {
    for(unsigned int i=0; i<npks; i++) {
        printf("[%u]\tc,s = %g(%g) +- %g(%g)\th = %g(%g)\n", i,
            getParameter(3*i+1), getParError(3*i+1),
            getParameter(3*i+2), getParError(3*i+2),
            getParameter(3*i+0), getParError(3*i+0));
    }
}

void MultiGaus::addCorrelated(unsigned int n, double relCenter, double relHeight, double relWidth) {
    corrPeak p;
    p.mainPeak = n;
    p.relCenter = relCenter;
    p.relWidth = relWidth?relWidth:sqrt(relCenter);
    p.relHeight = relHeight;
    corrPeaks.push_back(p);
}


double MultiGaus::operator() (double* x, double* par) {

    bool reject = true;
    for(unsigned int i=0; i<npks; i++) {
        if( iguess[3*i+1] - 1.01*nSigma*iguess[3*i+2] < x[0] && x[0] < iguess[3*i+1] + 1.01*nSigma*iguess[3*i+2] ) {
            reject = false;
            break;
        }
    }
    if(reject) {
        TF1::RejectPoint();
        return 0;
    }

    Double_t s = 0;
    for(unsigned int i=0; i<npks; i++)
        s += par[3*i]*exp( -(x[0]-par[3*i+1])*(x[0]-par[3*i+1])/(2*par[3*i+2]*par[3*i+2]) );
    for(auto const& pk: corrPeaks) {
        unsigned int i = pk.mainPeak;
        s += par[3*i]*pk.relHeight*exp( -(x[0]-par[3*i+1]*pk.relCenter)*(x[0]-par[3*i+1]*pk.relCenter)/(2*par[3*i+2]*par[3*i+2]*pk.relWidth) );
    }
    return s;
}

int iterGaus(TH1* h0, TF1* gf, unsigned int nit, float mu, float sigma, float nsigma, float asym) {
    int err = h0->Fit(gf,"Q","",mu-(nsigma-asym)*sigma,mu+(nsigma+asym)*sigma);
    if(!err && !nit)
        return iterGaus(h0,gf,nit-1,gf->GetParameter(1),gf->GetParameter(2),nsigma,asym);
    return err;
}
