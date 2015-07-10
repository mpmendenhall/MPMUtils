/// \file EnumerationFitter.hh \brief Least-squares fit to enumerated values
/* 
 * EnumerationFitter.hh, part of the MPMUtils package.
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


#ifndef ENUMERATIONFITTER_HH
#define ENUMERATIONFITTER_HH 1

#include <vector>
#include <string>
#include <TF1.h>
#include <TGraphErrors.h>
using std::vector;
using std::string;

/// Least-squares fit to enumerated values
class EnumerationFitter {
public:
    /// constructor
    EnumerationFitter(): fitter(NULL) {}
    /// destructor
    ~EnumerationFitter() { if(!fitter) delete fitter; }
    /// add a fit terms set
    void addTerm(const vector<double>& t);
    /// fit evaluation from sum of terms
    double Evaluate(double *x, double *p);
    /// get number of fit parameters
    unsigned int getNParams() const { return fterms.size(); }
    /// get fitter
    TF1* getFitter();
    /// load fittable data and terms from a file
    TGraphErrors* loadFitFile(const string& fname);
    
protected:
    
    vector< vector<double> > fterms;    ///< fit term sets
    TF1* fitter;                        ///< fitter based on these terms
};


#endif
