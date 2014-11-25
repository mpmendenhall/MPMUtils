/* 
 * LinMin.hh, part of the RotationShield program
 * Copyright (c) 2007-2014 Michael P. Mendenhall
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

/// \file "linmin.hh" \brief Least-squares linear polynomial fits
#ifndef LINMIN_HH
/// Make sure this header is included only once
#define LINMIN_HH

#include "gsl/gsl_matrix.h"
#include "gsl/gsl_vector.h"
#include <iostream>
#include "Polynomial.hh"

/// least-squares minimize coeffs*x = rslt+resid using QR decomposition. *frees* coeffs; needs proper-sized resid, returns x.
gsl_vector* lsmin(gsl_matrix* coeffs, const gsl_vector* rslt, gsl_vector* resid);
/// Linear fit to 3-variate polynomial; return rms residual. Coords is an N*3 matrix for N values at locations x,y,z
double polynomialFit(const gsl_matrix* coords, const gsl_vector* values, Polynomial<3,double>& p);

/// helper class for solving system of linear equations Mx = y+r
class LinEqSolver {
public:
    /// Constructor, for m equations in n variables
    LinEqSolver(size_t mm, size_t nn);
    /// Destructor
    ~LinEqSolver();
    /// set y
    void sety(size_t i, double v);
    /// set M
    void setM(size_t i, size_t j, double v);
    /// calculate solution x, r
    void solve();
    /// get sum of squares of residuals
    double ssresid() const;
    
    const size_t m;
    const size_t n;
    
    gsl_matrix* M = NULL;
    gsl_vector* x = NULL;
    gsl_vector* y = NULL;
    gsl_vector* r = NULL;
};

#endif
