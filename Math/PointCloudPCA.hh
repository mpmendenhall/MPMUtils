/// \file PointCloudPCA.hh Principal Components Analysis for weighted point cloud
// Michael P. Mendenhall, LLNL 2018

#ifndef POINTCLOUDPCA_HH
#define POINTCLOUDPCA_HH

#include <vector>
using std::vector;
#include <cstddef> // for size_t
#include <stdio.h>
#include <cmath>
#include <array>
#include <TMatrixD.h>
#include <TMatrixDSym.h>
#include <TMatrixDSymEigen.h>
#include <cassert>

/// point with weight in point cloud
template<int N>
struct weightedpt {
    /// Constructor
    weightedpt(double* xx = nullptr, double ww=1): w(ww) {
        if(xx) std::copy(xx,xx+N,x);
        else std::fill(xx,xx+N,0.);
    }

    double x[N];    ///< coordinate
    double w = 1;   ///< weight
};

/// PCA calculation
template<int N>
class PointCloudPCA {
public:
    /// Default constructor
    PointCloudPCA() { }

    /// Constructor, calculated from points
    PointCloudPCA(const vector<weightedpt<N>>& v): n(v.size()) {
        // weights vector
        TMatrixD w(1,n);
        for(size_t i=0; i<n; i++) { w(0,i) = v[i].w; }
        sw = w.Sum();

        // points matrix
        TMatrixD M(n,N);
        for(size_t i=0; i<n; i++) for(int j = 0; j<N; j++) M(i,j) = v[i].x[j];

        // calculate mean
        auto u = w*M;
        u *= 1./sw;
        for(int j = 0; j<N; j++) mu[j] = u(0,j);

        // subtract mean and apply weighting
        for(size_t i=0; i<n; i++) {
            double rw = sqrt(fabs(w(0,i)));
            for(int j = 0; j<N; j++) M(i,j) = (M(i,j)-mu[j])*rw;
        }

        // weighted covariance matrix
        TMatrixDSym MTM(TMatrixDSym::kAtA, M);
        for(int i = 0; i<N; i++) for(int j = 0; j<N; j++) Cov[i][j] = MTM(i,j);

        calcPrincipalComponents(MTM);
    }

    /// mean square spread along principal components direction
    inline double sigma2(int a) const { return width2[a]/sw; }
    /// rms spread along principal components direction
    inline double sigma(int a) const { return sqrt(sigma2(a)); }
    /// transverse width^2 from principal axis
    inline double wT2() const { double s = 0; for(int i = 0; i<N-1; i++) s += width2[i]; return s; }
    /// transverse spread from principal axis
    inline double sigmaT2() const { return wT2()/sw; }


    /// reverse direction
    void flip() {for(int i = 0; i<N-1; i++) for(int j = 0; j<N; j++) PCA[i][j] *= -1; }

    double mu[N];       ///< mean center
    double Cov[N][N];   ///< covariance matrix
    double PCA[N][N];   ///< orthogonal principal components vectors in PCA[i], largest to smallest
    double width2[N];   ///< spread along principal directions (eigenvalues of Cov), largest to smallest
    size_t n;           ///< number of points
    double sw;          ///< sum of weights

    void operator+=(const PointCloudPCA<N>& P) {
        if(!sw) {
            *this = P;
            return;
        }

        PointCloudPCA<N> Pnew;

        Pnew.n = n + P.n;
        Pnew.sw = sw + P.sw;
        for(int j = 0; j<N; j++) Pnew.mu[j] = (mu[j]*sw + P.mu[j]*P.sw)/Pnew.sw;

        for(int i = 0; i<N-1; i++) {
            for(int j = 0; j<N; j++) {
                Pnew.Cov[i][j] = Cov[i][j] + P.Cov[i][j] + (mu[i]-P.mu[i])*(mu[j]-P.mu[j])*sw*P.sw/Pnew.sw;
            }
        }

        *this = Pnew;
        calcPrincipalComponents(TMatrixDSym(N,Cov[0]));
    }

    /// out-of-place sum
    const PointCloudPCA operator+(const PointCloudPCA& P) const { auto PP = *this; PP += P; return PP; }

    void calcPrincipalComponents(const TMatrixDSym& MCov) {
        const TMatrixDSymEigen eigen(MCov);
        const TVectorD& eigenVal = eigen.GetEigenValues();
        const TMatrixD& eigenVec = eigen.GetEigenVectors();
        for(int i = 0; i<N; i++) {
            width2[i] = eigenVal[i];
            for(int j = 0; j<N; j++) PCA[j][i] = eigenVec(i,j);
        }
    }

    void display() const {
        printf("Cloud of %zu %i-points (average weight %g):\n", n, N, n? sw/n : 0);
        if(N==3) {
            printf("\tmean = %g\t%g\t%g\t\twidth2 = %g\t%g\t%g\n",
            mu[0], mu[1], mu[2], width2[0], width2[1], width2[2]);
            for(int i=0; i<3; i++) {
                printf("\t%g\t%g\t%g\t\t%g\t%g\t%g\n",
                    Cov[i][0], Cov[i][1], Cov[i][2],
                PCA[i][0], PCA[i][1], PCA[i][2] );
            }
        }
    }
};

#endif
