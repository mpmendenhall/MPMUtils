/// \file PointCloudPCA.hh Principal Components Analysis for weighted point cloud
// Michael P. Mendenhall, LLNL 2018

#ifndef POINTCLOUDPCA_HH
#define POINTCLOUDPCA_HH

#include <vector>
using std::vector;
#include <cstddef> // for size_t
#include <cmath>
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
    }
    double x[N];    ///< position
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

    /// rms spread along principal components direction
    double sigma(int a) const { return sqrt(width2[a]/sw); }

    double mu[N];       ///< mean center
    double Cov[N][N];   ///< covariance matrix
    double PCA[N][N];   ///< orthogonal principal components vectors in PCA[i], largest to smallest
    double width2[N];   ///< spread along principal directions (eigenvalues of Cov), largest to smallest
    size_t n;           ///< number of points
    double sw;          ///< sum of weights

    void calcPrincipalComponents(const TMatrixDSym& Cov) {
        const TMatrixDSymEigen eigen(Cov);
        const TVectorD& eigenVal = eigen.GetEigenValues();
        const TMatrixD& eigenVec = eigen.GetEigenVectors();
        for(int i = 0; i<N; i++) {
            width2[i] = eigenVal[i];
            for(int j = 0; j<N; j++) PCA[j][i] = eigenVec(i,j);
        }
    }
};

#endif
