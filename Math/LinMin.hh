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

#include "LinalgHelpers.hh"

/// helper class for solving (overdetermined) system of linear equations Mx = y + r
// Internally: decomposes M = Q (orthogonal) * R (right-triangular)
class LinMin: protected EigSymmWorkspace {
public:
    /// Constructor, for n variables with m equations
    LinMin(size_t nvar, size_t neq = 0): EigSymmWorkspace(nvar), Nvar(nvar) { setNeq(neq); }

    /// set number of equations (resize M)
    void setNeq(size_t neq);
    /// get Neq
    size_t nEq() const { return Neq; }
    /// get number of degrees of freedom
    size_t nDF() const { return Neq-Nvar; }
    /// clear previous calculation inputs
    virtual void clear() { has_tau = has_Cov = has_PCA = false; if(M) gsl_matrix_set_zero(M); }

    /// set element of M
    void setM(size_t i, size_t j, double v) { gsl_matrix_set(M,i,j,v); }

    /// calculate solution x, r
    template<typename YVec>
    void solve(const YVec& vy) { vector2gsl(vy,y); _solve(); }

    /// get sum of squares of residuals ~ sigma^2 * nDF
    double ssresid() const;
    /// calculate and return UNNORMALIZED covariance matrix (sum X^T X)^-1 (needs sigma^2 scaling)
    const gsl_matrix_wrapper& calcCov();
    /// calculate, return unit eigenvectors of covariance matrix in columns
    const gsl_matrix_wrapper& calcPCA();
    /// return eigenvalues for PCA vectors
    const gsl_vector_wrapper& PCAlambda() { calcPCA(); return lPCA; }

    /// get solution x
    void getx(vector<double>& vx) const { gsl2vector(x,vx); }
    /// get realization of solution within covariance
    void getRealization(const vector<double>& vr, vector<double>& vx);
    /// get residuals r
    void getr(vector<double>& vr) const { gsl2vector(r,vr); }

protected:
    size_t Nvar;  ///< number of variables

    /// solve after loading 'y' vector
    virtual void _solve();
    /// calculate QR decomposition of M (stored in M, tau)
    void calcQR();

    size_t Neq = 0;             ///< number of equations

    gsl_matrix_wrapper M;       ///< coefficients (design) matrix -> QR decomp
    gsl_vector_wrapper tau;     ///< from QR decomposition of M
    gsl_matrix_wrapper Q;       ///< unpacked Q decomposition matrix
    gsl_matrix_wrapper R;       ///< unpacked R decomposition
    gsl_matrix_wrapper L;       ///< L = R^T square corner, Cholesky Decomp M^T M = L L^T
    gsl_matrix_wrapper Cov;     ///< Cov = (M^T M)^-1 = (L L^T)^-1
    gsl_matrix_wrapper PCA;     ///< normalized eigenvectors of Cov in columns (for random realizations)
    gsl_vector_wrapper lPCA;    ///< eigenvalues of Cov
    gsl_vector_wrapper x;       ///< solution vector
    gsl_vector_wrapper y;       ///< RHS vector
    gsl_vector_wrapper r;       ///< residuals vector

    bool has_tau = false;       ///< QR decomposition calculated?
    bool has_Cov = false;       ///< Covariance matrix calculated?
    bool has_PCA = false;       ///< PCA calculated?
};

#endif
