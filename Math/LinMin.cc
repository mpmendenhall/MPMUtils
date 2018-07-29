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
#include <stdio.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_blas.h>

void LinMin::resize(size_t neq, size_t nvar) {
    if(neq == Neq && nvar == Nvar) return;
    clear();
    if(!neq || !nvar) return;
    Neq = neq;
    Nvar = nvar;
    M = gsl_matrix_alloc(Neq,Nvar);
    r = gsl_vector_calloc(Neq);
}

void LinMin::clear() {
    Neq = Nvar = 0;
    if(M) gsl_matrix_free(M);
    if(tau) gsl_vector_free(tau);
    if(x) gsl_vector_free(x);
    if(y) gsl_vector_free(y);
    if(r) gsl_vector_free(r);
    M = nullptr;
    x = y = r = tau = nullptr;
}

void LinMin::setM(size_t i, size_t j, double v) {
    assert(M && i<Neq && j<Nvar);
    gsl_matrix_set(M,i,j,v);
}

void LinMin::_solve() {
    if(!(M && y && r)) throw;

    if(x && x->size != M->size2) {
        gsl_vector_free(x);
        x = nullptr;
    }
    if(!x) x = gsl_vector_alloc(M->size2);

    // converts M to QR
    if(!tau) {
        tau = gsl_vector_alloc(M->size2);
        if(gsl_linalg_QR_decomp(M,tau)) throw;
    }

    if(gsl_linalg_QR_lssolve(M, tau, y, x, r)) throw;
}

double LinMin::ssresid() const {
    if(!r) return -1;
    return gsl_blas_dnrm2(r);
}

void gsl2vector(const gsl_vector* g, vector<double>& v) {
    if(!g) { v.clear(); return; }
    v.resize(g->size);
    for(size_t i=0; i<g->size; i++) v[i] = gsl_vector_get(g,i);
}

void LinMin::getx(vector<double>& vx) const { gsl2vector(x,vx); }

void LinMin::getr(vector<double>& vr) const { gsl2vector(r,vr); }
