/// \file "LinMin.hh" Least-squares linear equations solver
/*
 * LinMin.hh, part of the MPMUtils package.
 * Copyright (c) 2007-2018 Michael P. Mendenhall
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

#ifndef LINMIN_HH
#define LINMIN_HH

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>
#include <vector>
using std::vector;

/// helper class for solving system of linear equations Mx = y+r
class LinMin {
public:
    /// Constructor, for m equations in n variables
    LinMin(size_t neq = 0, size_t nvar = 0) { resize(neq, nvar); }
    /// Destructor
    ~LinMin() { clear(); }
    /// set M
    void setM(size_t i, size_t j, double v);
    /// calculate solution x, r
    void solve(const vector<double>& vy);

    /// get sum of squares of residuals
    double ssresid() const;
    /// get solution x
    void getx(vector<double>& vx) const;
    /// get resid r
    void getr(vector<double>& vr) const;

    /// resize to specified dimensions
    void resize(size_t neq, size_t nvar);
    /// clear solution, free data
    void clear();

protected:
    size_t Neq = 0;             ///< number of equations
    size_t Nvar = 0;            ///< number of variables

    gsl_matrix* M = nullptr;    ///< coefficients matrix
    gsl_vector* tau = nullptr;  ///< from QR decomposition of M
    gsl_vector* x = nullptr;    ///< solution vector
    gsl_vector* y = nullptr;    ///< RHS vector
    gsl_vector* r = nullptr;    ///< residuals vector
};

#endif
