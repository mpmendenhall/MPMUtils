/// @file "LinMinConstrained.hh" Least-squares linear solver with linear constraints
/*
 * LinMinConstrained.hh, part of the MPMUtils package.
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

#ifndef LINMINCONSTRAINED_HH
#define LINMINCONSTRAINED_HH

#include "LinMin.hh"

/// helper class for solving constrained (overdetermined) system of linear equations Mx = y + resid | Gx = k
class LinMinConstrained: public LinMin {
public:
    /// Constructor, for m equations in n variables
    explicit LinMinConstrained(size_t nvar, size_t neq = 0, size_t ncon = 0): LinMin(nvar, neq), Ncon(ncon) { }

    /// set number of constraints
    void setNConstraints(size_t nc);
    /// set constraints
    void setG(size_t i, size_t j, double v);
    /// set constraints RHS
    void setk(size_t i, double v);

    /// clear solution, free data
    void clear() override;
    /// clear constraints solution (permits re-evaluation with different constraints)
    void clear_constraints();

    /// get Lagrange Multipliers vector
    void getL(vector<double>& vl) const { gsl2vector(l,vl); }

protected:
    /// solve after loading 'y' vector
    void _solve() override;

    size_t Ncon;

    // for each M
    gsl_matrix_wrapper Q;       ///< Q from QR decomp
    gsl_matrix_wrapper RT;      ///< R^T from QR decomp (use with blas Cholesky Decomposition functions for R^T R)
    gsl_matrix_wrapper RTRi;    ///< (R^T R)^-1 = (M^T M)^-1 using Cholesky inverse

    // for each M,G
    gsl_matrix_wrapper G;       ///< Ncon*Nvar constraints matrix
    gsl_vector_wrapper k;       ///< RHS of constraints
    gsl_matrix_wrapper GRRM;    ///< G (R^T R)^-1 M^T reusable intermediate
    gsl_matrix_wrapper GRRG_CD; ///< Cholesky Decomposition for A*lambda = v solver

    // for each y,M,G
    gsl_vector_wrapper l;       ///< Lagrange Multipliers for constraints
};

#endif
