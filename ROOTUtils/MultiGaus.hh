/// \file MultiGaus.hh Multiple-Gaussian-peak fitter
/* 
 * MultiGaus.hh, part of the MPMUtils package.
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

#ifndef MULTIGAUS_HH
#define MULTIGAUS_HH 1

#include <string>
using std::string;
#include <vector>
using std::vector;

#include <climits>

#include <TF1.h>
#include <TH1F.h>
#include "FloatErr.hh"

/// class for fitting multi-peak gaussians
class MultiGaus {
public:
    
    /// correlated subpeaks specification
    struct corrPeak {
        unsigned int mainPeak;
        double relCenter;
        double relHeight;
        double relWidth;
    };
    
    /// constructor
    MultiGaus(unsigned int n, const string& name, float ns = 1.5): nSigma(ns), npks(n), iguess(new double[3*n]), myTF1(new TF1(name.c_str(),this,0,0,3*n)) { }
    
    /// destructor
    ~MultiGaus();
    
    /// add correlated peak
    void addCorrelated(unsigned int n, double relCenter, double relHeight, double relWidth = 0);
    /// fill initial values array
    void setParameter(unsigned int n, double p);
    /// set center,sigma initial values
    void setCenterSigma(unsigned int n, double c, double s);
        
    /// get fit parameter
    double getParameter(unsigned int n) const;
    /// get fit parameter error
    double getParError(unsigned int n) const;
    /// get parameter+error as float_err
    float_err getPar(unsigned int n) const;
    /// display fit results
    void display() const;
    
    /// get TF1 with appropriate pre-set values
    TF1* getFitter();
    
    /// single-peak fit estimate, assuming center and sigma initial guess set
    void fitEstimate(TH1* h, unsigned int n = INT_MAX);
    /// fit a TH1 after initial peak centers/widths have been guessed; update inital guess
    void fit(TH1* h, bool draw = true);
    
    /// gaussian evaluation function
    double operator() (double* x, double* par);
    
    float nSigma;               ///< number of sigma peak width to fit
    
    const unsigned int npks;    ///< number of peaks being fitted
    
protected:
    double* iguess;             ///< inital guess at peak positions
    TF1* myTF1;                 ///< TF1 using this class as its fit function
    vector<corrPeak> corrPeaks; ///< correlated subpeaks
};

int iterGaus(TH1* h0, TF1* gf, unsigned int nit, float mu, float sigma, float nsigma = 1.5, float asym = 0);

#endif
