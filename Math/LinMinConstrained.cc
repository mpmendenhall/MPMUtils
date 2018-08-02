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
#include <stdio.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_blas.h>

/*
Minimize |r^2| over x: M[Neq,Nvar] x = y + r,
    subject to additional linear constraints:
(0) G[Ncon,Nvar] x = k.

|r^2| = x^T M^T M x -2 y^T M x + y^T y
d|r^2|/dx = d/dx lambda G =>

(1) M^T M x = M^T y + G^T lambda

Decomposing M = Q[Neq,Neq] R[Neq,Nvar], where Q is orthogonal
    and R is upper-right-triangular (effectively [Nvar,Nvar]),
then M^T M = R^T R is square and symmetric:

(2) R^T R x = M^T y + G^T lambda
we can use Cholesky Decomposition inverse on R^T R

Substitute x = (RTR)^-1 (M^T y + G^T l) into (0) to solve for lambda(y,k):

(3) (G (RTR)^-1 G^T) lambda = k - (G (RTR)^-1 M^T) y
(solve for lambda using Cholesky Decomposition)

Substitute G^T lambda back into (2) to solve for x (using Cholesky Decomp. solver)
*/


void LinMinConstrained::clear() {
    LinMin::clear();
    clear_constraints();

    if(Q) gsl_matrix_free(Q);
    if(RTRi) gsl_matrix_free(RTRi);
    if(lambda) gsl_vector_free(lambda);
    Q = RTRi = nullptr;
    lambda = nullptr;
}

void LinMinConstrained::clear_constraints() {
    if(GRRM) gsl_matrix_free(GRRM);
    if(GRRG_CD) gsl_matrix_free(GRRG_CD);
    GRRM = GRRG_CD = nullptr;
}


void LinMinConstrained::_solve() {
    if(!(M && y && r)) throw;

    if(x && x->size != M->size2) {
        gsl_vector_free(x);
        x = nullptr;
    }
    if(!x) x = gsl_vector_alloc(M->size2);

    if(!tau) { // need QR decomposision

        tau = gsl_vector_alloc(M->size2);
        if(gsl_linalg_QR_decomp(M,tau)) throw;

        // Extract Q, R
        Q = gsl_matrix_alloc(Neq,Neq);
        auto R = gsl_matrix_alloc(Nvar,Nvar);
        if(gsl_linalg_QR_unpack(M, tau, Q, R)) throw;

        // RTRi = (R^T R)^-1
        // Transpose R to lower-triangular form assumed by gsl Cholesky
        if(gsl_matrix_transpose(R)) throw;
        RTRi = R; // renaming for inversion
        gsl_linalg_cholesky_invert(RTRi);

        clear_constraints(); // need new constraints solver
    }

    if(!GRRM) { // need constraints solver?

        auto GRR = gsl_matrix_alloc(Ncon, Nvar);
        // C = a BA + b C, symmetric A
        if(gsl_blas_dsymm(CblasRight,   // AB vs. BA
                          CblasLower,   // RTRi lower-triangle representation
                          1.0,          // a
                          G,            // A
                          RTRi,         // B
                          0.0,          // b
                          GRR)         // C
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
        if(gsl_linalg_cholesky_decomp1(GRRG_CD)) throw;

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
