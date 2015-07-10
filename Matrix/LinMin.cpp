/// \file LinMin.cpp
/* 
 * LinMin.cpp, part of the MPMUtils package.
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

#include "LinMin.hh"
#include <cassert>
#include "gsl/gsl_linalg.h"
#include "gsl/gsl_blas.h"

gsl_vector* lsmin(gsl_matrix* coeffs, const gsl_vector* rslt, gsl_vector* resid) {
    
    assert(coeffs && rslt && resid);
    assert(coeffs->size1 >= coeffs->size2);
    assert(resid->size == coeffs->size1);
    assert(rslt->size == coeffs->size1);
    
    gsl_vector* tau = gsl_vector_calloc(coeffs->size2);
    assert(!gsl_linalg_QR_decomp(coeffs,tau));
    gsl_vector* x = gsl_vector_calloc(coeffs->size2);
    assert(!gsl_linalg_QR_lssolve(coeffs, tau, rslt, x, resid));
    
    gsl_vector_free(tau);
    gsl_matrix_free(coeffs);
    return x;
}

double polynomialFit(const gsl_matrix* coords, const gsl_vector* values, Polynomial<3,double>& p) {
    int nparams = p.terms.size();
    assert(coords && values);
    assert((unsigned int)nparams <= values->size);
    assert(coords->size1 == values->size);
    assert(coords->size2 == 3);
    
    // build coefficients matrix
    gsl_matrix* coeffs = gsl_matrix_alloc(coords->size1,nparams);
    Vec<3,double> coord;
    p.it = p.terms.begin();
    for(int j=0; j<nparams; j++) {
        Monomial<3,double,unsigned int> m = Monomial<3,double,unsigned int>(1.0,p.it->first);
        for(unsigned int i=0; i<values->size; i++) {
            for(int c=0; c<3; c++) coord[c] = gsl_matrix_get(coords,i,c);
            gsl_matrix_set(coeffs,i,j,m(coord));
        }
        p.it++;
    }
    
    // fit, cleanup, return
    gsl_vector* resid = gsl_vector_calloc(values->size);
    gsl_vector* fitv = lsmin(coeffs,values,resid);
    p.it = p.terms.begin();
    for(int j=0; j<nparams; j++) {
        p.terms[p.it->first] = gsl_vector_get(fitv,j);
        p.it++;
    }
    double rsresid =  gsl_blas_dnrm2(resid);
    gsl_vector_free(fitv);
    gsl_vector_free(resid);
    return rsresid/sqrt(values->size);
}


LinEqSolver::LinEqSolver(size_t mm, size_t nn): m(mm), n(nn) {
    M = gsl_matrix_alloc(m,n);
    y = gsl_vector_calloc(m);
    r = gsl_vector_calloc(m);
}

LinEqSolver::~LinEqSolver() {
    if(M) delete M;
    if(x) delete x;
    if(y) delete y;
    if(r) delete r;
}

void LinEqSolver::sety(size_t i, double v) {
    assert(i<m);
    gsl_vector_set(y, i, v);
}

void LinEqSolver::setM(size_t i, size_t j, double v) {
    assert(M && i<m && j<n);
    gsl_matrix_set(M,i,j,v);
}

void LinEqSolver::solve() {
    if(!(M && y && r)) return;
    if(x) delete x;
    x = lsmin(M,y,r);
    M = NULL;
}

double LinEqSolver::ssresid() const {
    return gsl_blas_dnrm2(r);
}

double LinEqSolver::getx(size_t i) const { return x?gsl_vector_get(x,i):0; }
double LinEqSolver::getr(size_t i) const { return gsl_vector_get(r,i); }
