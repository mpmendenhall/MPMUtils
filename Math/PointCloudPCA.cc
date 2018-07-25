/// \file PointCloudPCA.cc

#include "PointCloudPCA.hh"
#include <stdio.h>
#include <TMatrixD.h>
#include <TMatrixDSym.h>
#include <TMatrixDSymEigen.h>
#include <cassert>

void weightedpt::display() const {
    printf("weightedpt (%g, %g, %g) w=%g\n", x[0], x[1], x[2], w);
}

//////////////////////////////
//////////////////////////////
//////////////////////////////

void calcPrincipalComponents(const TMatrixDSym& Cov, PointCloudPCA& P) {
    const TMatrixDSymEigen eigen(Cov);
    const TVectorD& eigenVal = eigen.GetEigenValues();
    const TMatrixD& eigenVec = eigen.GetEigenVectors();
    bool isOK = true;
    for(auto i: {0,1,2}) {
        P.width2[i] = eigenVal[i];
        if(!(P.width2[i] == P.width2[i])) isOK = false;
        for(auto j: {0,1,2}) P.PCA[j][i] = eigenVec(i,j);
    }
    if(!isOK) {
        for(auto i: {0,1,2}) {
            P.width2[i] = 0;
            for(auto j: {0,1,2}) P.PCA[j][i] = i==j;
        }
    }
}

PointCloudPCA::PointCloudPCA(const vector<weightedpt>& v): n(v.size()) {
    // weights vector
    TMatrixD w(1,n);
    size_t nplus = 0; // number of positive elements
    for(size_t i=0; i<n; i++) { w(0,i) = v[i].w; nplus += (v[i].w >= 0); }
    sw = w.Sum();

    // points matrix
    TMatrixD M(n,3);
    for(size_t i=0; i<n; i++)
        for(auto j: {0,1,2})
            M(i,j) = v[i].x[j];

    // calculate mean
    auto u = w*M;
    u *= 1./sw;
    for(auto j: {0,1,2}) mu[j] = u(0,j);

    if(nplus == n) {
        // subtract mean and apply weighting
        for(size_t i=0; i<n; i++) {
            double rw = sqrt(fabs(w(0,i)));
            for(auto j: {0,1,2}) M(i,j) = (M(i,j)-mu[j])*rw;
        }

        // weighted covariance matrix
        TMatrixDSym MTM(TMatrixDSym::kAtA, M);
        for(auto i: {0,1,2}) for(auto j: {0,1,2}) Cov[i][j] = MTM(i,j);

        calcPrincipalComponents(MTM,*this);


    } else {

        // split into positive, negative-weight entries
        TMatrixD Mp(nplus,3);
        TMatrixD Mm(n-nplus,3);
        int ip = 0;
        int im = 0;

        for(size_t i=0; i<n; i++) {
            auto rw = sqrt(fabs(w(0,i)));
            if(w(0,i) >= 0) {
                for(auto j: {0,1,2}) Mp(ip,j) = (M(i,j)-mu[j])*rw;
                ip++;
            } else {
                for(auto j: {0,1,2}) Mm(im,j) = (M(i,j)-mu[j])*rw;
                im++;
            }
        }

        TMatrixDSym MTMp(TMatrixDSym::kAtA, Mp);
        TMatrixDSym MTMm(TMatrixDSym::kAtA, Mm);
        MTMp -= MTMm;
        for(auto i: {0,1,2}) for(auto j: {0,1,2}) Cov[i][j] = MTMp(i,j);
        calcPrincipalComponents(MTMp, *this);
    }
}

void PointCloudPCA::flip() {
    for(auto i: {0,1,2}) for(auto j: {0,1,2}) PCA[i][j] *= -1;
}

void PointCloudPCA::operator+=(const PointCloudPCA& P) {
    PointCloudPCA Pnew;

    Pnew.n = n + P.n;
    Pnew.sw = sw + P.sw;
    for(auto j: {0,1,2}) Pnew.mu[j] = (mu[j]*sw + P.mu[j]*P.sw)/Pnew.sw;

    for(auto i: {0,1,2}) {
        for(auto j: {0,1,2}) {
            Pnew.Cov[i][j] = Cov[i][j] + P.Cov[i][j] + (mu[i]-P.mu[i])*(mu[j]-P.mu[j])*sw*P.sw/Pnew.sw;
        }
    }

    *this = Pnew;
    recalc();
}

void PointCloudPCA::recalc() {
    calcPrincipalComponents(TMatrixDSym(3,Cov[0]), *this);
}

void PointCloudPCA::display() const {
    printf("Cloud of %zu points (average weight %g):\n", n, sw/n);
    printf("\tmean = %g\t%g\t%g\t\twidth2 = %g\t%g\t%g\n",
           mu[0], mu[1], mu[2],
           width2[0], width2[1], width2[2]);
    for(int i=0; i<3; i++) {
        printf("\t%g\t%g\t%g\t\t%g\t%g\t%g\n",
               Cov[i][0], Cov[i][1], Cov[i][2],
               PCA[i][0], PCA[i][1], PCA[i][2] );
    }
}
