/// \file LinMinConstrained.cc
/*
 * LinMinConstrained.cc, part of the MPMUtils package.
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

#include "LinMinConstrained.hh"
#include <cassert>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_blas.h>
#include <iostream>

/*
Minimize |r^2| over x: M[Neq,Nvar] x = y + r,
    subject to additional linear constraints:
(0) G[Ncon,Nvar] x = k.

|r^2| = x^T M^T M x -2 y^T M x + y^T y
d|r^2|/dx = d/dx lambda G =>

(1) M^T M x = M^T y + G^T lambda

Decomposing M = Q[Neq,Neq] R[Neq,Nvar], where Q is orthogonal
    and R is upper-right-triangular;
    let S[Nvar,Nvar] be the truncated upper square of R, then
then M^T M = R^T R = S^T S is square and symmetric.

(2) S^T S x = M^T y + G^T lambda
we can use Cholesky Decomposition inverse on S^T S

Substitute x = (S^T S)^-1 (M^T y + G^T l) into (0) to solve for lambda(y,k):

(3) (G (S^T S)^-1 G^T) lambda = k - (G (S^T S)^-1 M^T) y
(solve for lambda using Cholesky Decomposition)

Substitute G^T lambda back into (2) to solve for x (using Cholesky Decomp. solver)
*/


void LinMinConstrained::clear() {
    LinMin::clear();
    clear_constraints();

    if(Q) gsl_matrix_free(Q);
    if(RT) gsl_matrix_free(RT);
    if(RTRi) gsl_matrix_free(RTRi);
    Q = RT = RTRi = nullptr;
}

void LinMinConstrained::setG(size_t i, size_t j, double v) {
    assert(G && i<Ncon && j<Nvar);
    gsl_matrix_set(G,i,j,v);
}

void LinMinConstrained::setk(size_t i, double v) {
    assert(k && i<Ncon);
    gsl_vector_set(k,i,v);
}

void LinMinConstrained::clear_constraints() {
    if(G) gsl_matrix_free(G);
    if(GRRM) gsl_matrix_free(GRRM);
    if(GRRG_CD) gsl_matrix_free(GRRG_CD);
    if(k) gsl_vector_free(k);
    if(l) gsl_vector_free(l);
    G = GRRM = GRRG_CD = nullptr;
    k = l = nullptr;
}

void LinMinConstrained::setNConstraints(size_t nc) {
    Ncon = nc;
    if(!G || G->size1 != Ncon || G->size2 != Nvar) clear_constraints();
    if(!Ncon) return;
    G = gsl_matrix_calloc(Ncon,Nvar);
    resize(k,Ncon);
}

void LinMinConstrained::_solve() {
    if(!G) { LinMin::_solve(); return; }
    if(!(M && y && r)) throw;

    resize(x,M->size2);

    if(!tau) { // need QR decomposision?

        tau = gsl_vector_alloc(M->size2);
        if(gsl_linalg_QR_decomp(M,tau)) throw;

        // Extract Q, R
        Q = gsl_matrix_alloc(Neq,Neq);
        auto R0 = gsl_matrix_alloc(Neq,Nvar);
        if(gsl_linalg_QR_unpack(M, tau, Q, R0)) throw;

        // Truncate, transpose R to lower-triangular form assumed by gsl Cholesky
        auto R = gsl_matrix_alloc(Nvar,Nvar);
        for(size_t i=0; i<Nvar; i++) for(size_t j=0; i<Nvar; i++) gsl_matrix_set(R,i,j,gsl_matrix_get(R0,i,j));
        if(gsl_matrix_transpose(R)) throw;
        // RTRi = (R^T R)^-1
        RTRi = R; // renaming for inversion
        gsl_linalg_cholesky_invert(RTRi);

        gsl_matrix_free(R0);
        clear_constraints(); // need new constraints solver
    }

    if(!GRRM) { // need constraints solver?

        //gsl_matrix_print(RTRi);

        auto GRR = gsl_matrix_alloc(Ncon, Nvar);
        // C = a BA + b C, symmetric A
        if(gsl_blas_dsymm(CblasRight,   // AB vs. BA
                          CblasLower,   // RTRi lower-triangle representation
                          1.0,          // a
                          RTRi,         // A
                          G,            // B
                          0.0,          // b
                          GRR)          // C
          ) throw;

        GRRG_CD = gsl_matrix_alloc(Ncon,Ncon);
        if(gsl_blas_dgemm(CblasNoTrans, // op(A)
                          CblasTrans,   // op(B)
                          1.0,          // a
                          GRR,         // A
                          G,            // B
                          0.0,          // b
                          GRRG_CD)      // C
          ) throw;
        if(gsl_linalg_cholesky_decomp(GRRG_CD)) throw;      // older deprecated function call
        //if(gsl_linalg_cholesky_decomp1(GRRG_CD)) throw;   // newer GSL call

        GRRM = gsl_matrix_alloc(Ncon,Neq);
        if(gsl_blas_dgemm(CblasNoTrans, // op(A)
                          CblasTrans,   // op(B)
                          1.0,          // a
                          GRR,          // A
                          M,            // B
                          0.0,          // b
                          GRRM)         // C
        ) throw;
        gsl_matrix_free(GRR);
    }

    // calculate u = k - (G (R^T R)^-1 M^T) y
    auto u = gsl_vector_alloc(Ncon);
    gsl_vector_memcpy(u,k);
    // y = a op(A) x + b y
    if(gsl_blas_dgemv(CblasNoTrans, // op(A)
                      -1.0,         // a
                      GRRM,         // A
                      y,            // x
                      1.0,          // b
                      u)            // y
      ) throw;

    // solve (G RTR^-1 G^T) l = u for l
    if(gsl_linalg_cholesky_solve(GRRG_CD, u, l)) throw;
    gsl_vector_free(u);

    // calculate v = M^T y + G^T l
    auto v = gsl_vector_alloc(Neq);
    // y = a op(A) x + b y
    if(gsl_blas_dgemv(CblasTrans,   // op(A)
                      1.0,          // a
                      G,            // A
                      l,            // x
                      0,            // b
                      v)            // y
      ) throw;
    if(gsl_blas_dgemv(CblasTrans,   // op(A)
                      1.0,          // a
                      M,            // A
                      y,            // x
                      1.0,          // b
                      v)            // y
      ) throw;

    // solve for x
    if(gsl_linalg_cholesky_solve(RT, v, x)) throw;
    gsl_vector_free(v);
}

