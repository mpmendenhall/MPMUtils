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
#include <gsl/gsl_eigen.h>
#include <vector>
using std::vector;

/// helper class for solving (overdetermined) system of linear equations Mx = y + r
// Internally: decomposes M = Q (orthogonal) * R (right-triangular)
class LinMin {
public:
    /// Constructor, for m equations in n variables
    LinMin(size_t neq = 0, size_t nvar = 0) { resize(neq, nvar); }
    /// Destructor
    virtual ~LinMin() { clear(); }
    /// get Nvar
    size_t nVar() const { return Nvar; }
    /// get Neq
    size_t nEq() const { return Neq; }

    /// set element of M
    void setM(size_t i, size_t j, double v);

    /// calculate solution x, r
    template<typename YVec>
    void solve(const YVec& vy) { vector2gsl(vy,y); _solve(); }

    /// get sum of squares of residuals
    double ssresid() const;
    /// calculate and return covariance matrix
    const gsl_matrix* calcCov();
    /// calculate, return unit eigenvectors of covariance matrix in columns
    const gsl_matrix* calcPCA();
    /// return eigenvalues for PCA vectors
    const gsl_vector* PCAlambda() { calcPCA(); return lPCA; }



    /// get solution x
    void getx(vector<double>& vx) const { gsl2vector(x,vx); }
    /// get realization of solution within covariance
    void getRealization(const vector<double>& vr, vector<double>& vx);
    /// get resid r
    void getr(vector<double>& vr) const { gsl2vector(r,vr); }

    /// resize to specified dimensions
    virtual void resize(size_t neq, size_t nvar);
    /// clear solution, free data
    virtual void clear();

    /// allocate and/or re-size gsl vector
    static void resize(gsl_vector*& g, size_t n);

    /// fill gsl vector
    template<typename YVec>
    static void vector2gsl(const YVec& v, gsl_vector*& g) {
        resize(g, v.size());
        for(size_t i=0; i<g->size; i++) gsl_vector_set(g,i,v[i]);
    }
    /// extract gsl vector
    template<typename Vec>
    static void gsl2vector(const gsl_vector* g, Vec& v) {
        if(!g) { v.clear(); return; }
        v.resize(g->size);
        for(size_t i=0; i<g->size; i++) v[i] = gsl_vector_get(g,i);
    }

protected:
    /// solve after loading 'y' vector
    virtual void _solve();
    /// calculate QR decomposition of M (stored in M, tau)
    void calcQR();

    size_t Neq = 0;             ///< number of equations
    size_t Nvar = 0;            ///< number of variables

    gsl_matrix* M = nullptr;    ///< coefficients (design) matrix -> QR decomp
    gsl_vector* tau = nullptr;  ///< from QR decomposition of M
    gsl_matrix* Q = nullptr;    ///< unpacked Q decomposition matrix
    gsl_matrix* R = nullptr;    ///< unpacked R decomposition
    gsl_matrix* L = nullptr;    ///< L = R^T square corner, Cholesky Decomp M^T M = L L^T
    gsl_matrix* Cov = nullptr;  ///< Cov = (M^T M)^-1 = (L L^T)^-1
    gsl_matrix* PCA = nullptr;  ///< normalized eigenvectors of Cov in columns (for random realizations)
    gsl_vector* lPCA = nullptr; ///< eigenvalues of Cov
    gsl_vector* x = nullptr;    ///< solution vector
    gsl_vector* y = nullptr;    ///< RHS vector
    gsl_vector* r = nullptr;    ///< residuals vector
    gsl_eigen_symmv_workspace* esw = nullptr;   ///< eigendecomposition workspace

};

#endif
