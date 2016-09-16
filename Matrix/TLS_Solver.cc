/// \file TLS_Solver.cc
// This file was produced under the employ of the United States Government,
// and is consequently in the PUBLIC DOMAIN, free from all provisions of
// US Copyright Law (per USC Title 17, Section 105).
//
// -- Michael P. Mendenhall, 2015

#include "TLS_Solver.hh"

TLS_Solver::TLS_Solver(size_t nn, size_t mm): n(nn), B(mm,n), mu(n), v(n) {
}

TLS_Solver::~TLS_Solver() {
    delete mySVD;
}

void TLS_Solver::solve() {
    // mean value
    mu = B.getColSum()*(1./B.nRows());
    for(size_t mm = 0; mm < B.nRows(); mm++)
        for(size_t nn = 0; nn < n; nn++)
            B(mm,nn) -= mu[nn];

    // SVD
    delete mySVD;
    VarMat<double> BB = B;
    mySVD = new LAPACKE_Matrix_SVD<double,double>(BB);
    v = mySVD->getRightSVec(0);
}

double TLS_Solver::getSSR() const {
    return B.getSumSquares() - (B*v).mag2();
}

