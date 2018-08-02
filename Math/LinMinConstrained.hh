/// \file "LinMinConstrained.hh" Least-squares linear solver with linear constraints
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
    LinMinConstrained(size_t neq = 0, size_t nvar = 0, size_t ncon = 0): LinMin(neq, nvar), Ncon(ncon) { }

    /// set number of constraints
    void setNConstraints(size_t nc) { /*TODO*/ }
    /// set constraints
    void setG(size_t i, size_t j, double v) {/*TODO*/ }
    /// set constraints RHS
    void setk(const vector<double>& rhs) { /*k = rhs;*/ }

    /// clear solution, free data
    void clear() override;
    /// clear constraints solution (permits re-evaluation with different constraints)
    void clear_constraints();

    /// get Lagrange Multipliers vector
    //void getL(vector<double>& l) const { LM.getx(l); }

protected:
    /// solve after loading 'y' vector
    void _solve() override;

    size_t Ncon;

    gsl_matrix* Q = nullptr;        ///< Q from QR decomp
    gsl_matrix* RT = nullptr;       ///< R^T from QR decomp (use with blas Cholesky Decomposition functions for R^T R)
    gsl_matrix* RTRi = nullptr;     ///< (R^T R)^-1 = (M^T M)^-1 using Cholesky inverse

    gsl_matrix* G = nullptr;        ///< Ncon*Nvar constraints matrix
    gsl_vector* k = nullptr;        ///< RHS of constraints
    gsl_vector* l = nullptr;        ///< Lagrange Multipliers for constraints

    gsl_matrix* GRRM  = nullptr;    ///< G (R^T R)^-1 M^T reusable intermediate
    gsl_matrix* GRRG_CD = nullptr;  ///< Cholesky Decomposition for A*lambda = v solver
    gsl_vector* lambda = nullptr;   ///< Lagrange Multipliers
};

#endif
