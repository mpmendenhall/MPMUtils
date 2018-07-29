/// \file LinMin.cc
/*
 * LinMin.cc, part of the MPMUtils package.
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

#include "LinMin.hh"
#include <cassert>
#include "gsl/gsl_linalg.h"
#include "gsl/gsl_blas.h"

/*
double polynomialFit(const gsl_matrix* coords, const gsl_vector* values, Polynomial<3,double>& p) {
    int nparams = p.terms.size();
    assert(coords && values);
    assert((unsigned int)nparams <= values->size);
    assert(coords->size1 == values->size);
    assert(coords->size2 == 3);

    // build coefficients matrix
    gsl_matrix* coeffs = gsl_matrix_alloc(coords->size1,nparams);
    Vec<3,double> coord;
    auto pit = p.terms.begin();
    for(int j=0; j<nparams; j++) {
        Monomial<3,double,unsigned int> m = Monomial<3,double,unsigned int>(1.0,pit->first);
        for(unsigned int i=0; i<values->size; i++) {
            for(int c=0; c<3; c++) coord[c] = gsl_matrix_get(coords,i,c);
            gsl_matrix_set(coeffs,i,j,m(coord));
        }
        pit++;
    }

    // fit, cleanup, return
    gsl_vector* resid = gsl_vector_calloc(values->size);
    gsl_vector* fitv = lsmin(coeffs,values,resid);
    pit = p.terms.begin();
    for(int j=0; j<nparams; j++) {
        p.terms[pit->first] = gsl_vector_get(fitv,j);
        pit++;
    }
    double rsresid =  gsl_blas_dnrm2(resid);
    gsl_vector_free(fitv);
    gsl_vector_free(resid);
    return rsresid/sqrt(values->size);
}
*/

void LinEqSolver::resize(size_t neq, size_t nvar) {
    if(neq == Neq && nvar == Nvar) return;
    clear();
    if(!neq || !nvar) return;
    Neq = neq;
    Nvar = nvar;
    M = gsl_matrix_alloc(Neq,Nvar);
    y = gsl_vector_calloc(Neq);
    r = gsl_vector_calloc(Neq);
}

void LinEqSolver::clear() {
    Neq = Nvar = 0;
    if(M) gsl_matrix_free(M);
    if(x) gsl_vector_free(x);
    if(y) gsl_vector_free(y);
    if(r) gsl_vector_free(r);
    M = nullptr;
    x = y = r = nullptr;
}

void LinEqSolver::sety(size_t i, double v) {
    assert(i<Neq);
    gsl_vector_set(y, i, v);
}

void LinEqSolver::setM(size_t i, size_t j, double v) {
    assert(M && i<Neq && j<Nvar);
    gsl_matrix_set(M,i,j,v);
}

void LinEqSolver::solve() {
    if(!(M && y && r)) return;
    if(x) gsl_vector_free(x);

    gsl_vector* tau = gsl_vector_calloc(M->size2);
    if(gsl_linalg_QR_decomp(M,tau)) throw;
    x = gsl_vector_calloc(M->size2);
    if(gsl_linalg_QR_lssolve(M, tau, y, x, r));

    gsl_vector_free(tau);
    gsl_matrix_free(M);
    M = nullptr;
}

double LinEqSolver::ssresid() const {
    return gsl_blas_dnrm2(r);
}

void LinEqSolver::getx(vector<double>& vx) const {
    if(!x) vx.clear();
    //auto p0 = &gsl_vector_get(x,0);
    //vx.assign(p0, p0+Nvar);
}

void LinEqSolver::getr(vector<double>& vr) const {
    if(!r) vr.clear();
    //auto p0 = &gsl_vector_get(r,0);
    //vr.assign(p0, p0+Neq);
}
