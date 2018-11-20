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
#include "LinalgHelpers.hh"
#include <cassert>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_blas.h>

void LinMin::setNeq(size_t neq) {
    clear();
    if(neq == Neq) return;

    Neq = neq;
    if(M) { gsl_matrix_free(M); M = nullptr; }

    if(!Neq) return;
    M = gsl_matrix_alloc(Neq, Nvar);
}

LinMin::~LinMin() {
    for(auto& m: {M,Q,R,L,Cov,PCA}) if(m) gsl_matrix_free(m);
    for(auto& v: {tau,x,y,r,lPCA}) if(v) gsl_vector_free(v);
}

void LinMin::calcQR() {
    if(has_tau) return; // already done
    if(!tau) tau = gsl_vector_alloc(M->size2);
    gsl_linalg_QR_decomp(M,tau);
    has_tau = true;
}

void LinMin::_solve() {
    calcQR();
    resize(x, M->size2);
    resize(r, M->size1);
    gsl_linalg_QR_lssolve(M, tau, y, x, r);
}

const gsl_matrix* LinMin::calcCov() {
    if(has_Cov) return Cov; // already calculated
    calcQR();

    // unpack Q,R decomposition matrices
    Q = gsl_matrix_alloc(Neq, Neq);
    R = gsl_matrix_alloc(Neq, Nvar);
    gsl_linalg_QR_unpack(M, tau, Q, R);

    // copy to L
    L = gsl_matrix_calloc(Nvar, Nvar);
    for(size_t i = 0; i < Nvar; i++)
        for(size_t j = i; j < Nvar; j++)
            gsl_matrix_set(L, j, i, gsl_matrix_get(R, i, j));


    // Cov = (M^T M)^-1 = (L L^T)^-1
    Cov = gsl_matrix_alloc(Nvar, Nvar);
    gsl_matrix_memcpy(Cov, L);
    gsl_linalg_cholesky_invert(Cov);

    has_Cov = true;
    return Cov;
}

const gsl_matrix* LinMin::calcPCA() {
    if(has_PCA) return PCA;

    if(!PCA) PCA = gsl_matrix_alloc(Nvar, Nvar);
    if(!lPCA) lPCA = gsl_vector_alloc(Nvar);
    gsl_matrix_memcpy(PCA, calcCov());
    decompSymm(PCA, lPCA);

    has_PCA = true;
    return PCA;
}

void LinMin::getRealization(const vector<double>& vr, vector<double>& vx) {
    calcPCA();

    getx(vx);
    for(size_t i = 0; i < std::min(Nvar, vr.size()); i++)
        for(size_t j = 0; j < Nvar; j++)
            vx[j] += vr[i] * gsl_vector_get(lPCA, i) * gsl_matrix_get(PCA, i, j);
}

double LinMin::ssresid() const {
    if(!r) return -1;
    double ssr;
    gsl_blas_ddot(r,r,&ssr);
    return ssr;
}

