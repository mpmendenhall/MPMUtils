/// \file LinHistCombo.hh Least-squares fitter for linear combinations of histograms
/* 
 * LinHistCombo.hh, part of the MPMUtils package.
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

#ifndef LINHISTCOMBO_HH
#define LINHISTCOMBO_HH

#include <vector>
#include <string>
#include <TH1.h>
#include <TF1.h>

using std::string;
using std::vector;

/// Class for fitting with a linear combination of histograms
class LinHistCombo {
public:
    /// constructor
    LinHistCombo(): interpolate(true), myFit(nullptr) {}
    /// destructor
    ~LinHistCombo() { delete myFit; }
    /// add a fit term
    void addTerm(TH1* h) { terms.push_back(h); }
    /// get fitter
    TF1* getFitter();
    /// fit histogram with linear combination of terms
    int Fit(TH1* h, double xmin, double xmax, const std::string& fitopt = "QR");
    /// require coefficients to be non-negative
    void forceNonNegative();
    
    vector<double> coeffs;      ///< fit coefficients
    vector<double> dcoeffs;     ///< fit coefficient errors
    bool interpolate;           ///< whether to interpolate between bins
    
    /// fit evaluation
    double Evaluate(double* x, double* p);
    /// fit evaluation with current coefficients
    double eval(double x) { return Evaluate(&x, coeffs.data()); }

protected:
    TF1* myFit;                         ///< fit function
    vector<TH1*> terms;                 ///< fit terms
    static unsigned int nFitters;       ///< naming counter
};

#endif
