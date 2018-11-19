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
    for(auto& m: {M,Q,R,L,Cov,PCA}) if(m) gsl_matrix_free(m);
    M = Q = R = L = Cov = PCA = nullptr;
    for(auto& v: {tau,x,y,r,lPCA}) if(v) gsl_vector_free(v);
    tau = x = y = r = lPCA = nullptr;
    if(esw) gsl_eigen_symmv_free(esw);
    esw = nullptr;
}

void LinMin::resize(gsl_vector*& g, size_t n) {
    if(g && g->size != n) { gsl_vector_free(g); g = nullptr; }
    if(!g) g = gsl_vector_calloc(n);
}

void LinMin::setM(size_t i, size_t j, double v) {
    assert(M && i<Neq && j<Nvar);
    gsl_matrix_set(M,i,j,v);
}

void LinMin::calcQR() {
    if(tau) return; // already done
    if(!M) throw;

    tau = gsl_vector_alloc(M->size2);
    if(gsl_linalg_QR_decomp(M,tau)) throw;
}

void LinMin::_solve() {
    if(!(y && r)) throw;
    calcQR();
    resize(x, M->size2);
    if(gsl_linalg_QR_lssolve(M, tau, y, x, r)) throw;
}

const gsl_matrix* LinMin::calcCov() {
    if(Cov) return Cov; // already calculated
    calcQR();

    // unpack Q,R decomposition matrices
    Q = gsl_matrix_alloc(Neq, Neq);
    R = gsl_matrix_alloc(Neq, Nvar);
    if(gsl_linalg_QR_unpack(M, tau, Q, R)) throw;

    // copy to L
    L = gsl_matrix_calloc(Nvar, Nvar);
    for(size_t i = 0; i < Nvar; i++)
        for(size_t j = i; j < Nvar; j++)
            gsl_matrix_set(L, j, i, gsl_matrix_get(R, i, j));


    // Cov = (M^T M)^-1 = (L L^T)^-1
    Cov = gsl_matrix_alloc(Nvar, Nvar);

    // alternate method for comparison:
    //gsl_blas_dsyrk(CblasLower, CblasNoTrans, 1., L, 0., Cov);
    //gsl_linalg_cholesky_decomp1(Cov);

    if(gsl_matrix_memcpy(Cov, L)) throw;
    if(gsl_linalg_cholesky_invert(Cov)) throw;
    return Cov;
}

const gsl_matrix* LinMin::calcPCA() {
    if(PCA) return PCA;

    esw = gsl_eigen_symmv_alloc(Nvar);
    PCA = gsl_matrix_alloc(Nvar, Nvar);
    auto A = gsl_matrix_alloc(Nvar, Nvar);
    lPCA = gsl_vector_alloc(Nvar);
    if(gsl_matrix_memcpy(A, calcCov())) throw;
    if(gsl_eigen_symmv(A, lPCA, PCA, esw)) throw;
    gsl_matrix_free(A);

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

