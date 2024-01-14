/// @file PointCloudPCA.hh Principal Components Analysis for weighted point cloud
// -- Michael P. Mendenhall, LLNL 2021

#ifndef POINTCLOUDPCA_HH
#define POINTCLOUDPCA_HH

#include <stdexcept>
#include <vector>
using std::vector;
#include <cstddef> // for size_t
#include <stdio.h>
#include <cmath>
#include <array>
using std::array;

#include <TMatrixD.h>
#include <TMatrixDSym.h>
#include <TMatrixDSymEigen.h>

/// point with weight in point cloud
template<size_t N, typename T = double, typename W = double>
struct weightedpt: public array<T, N> {
    /// dimensionality
    static constexpr size_t NDIM = N;
    /// coordinate component
    typedef T component_t;
    /// coordinate
    typedef array<component_t, NDIM> coord_t;
    /// point weight
    typedef W weight_t;

    /// Default zero constructor
    weightedpt(): coord_t{{}}, w{} { }
    /// Weights-only constructor
    explicit weightedpt(weight_t ww): coord_t{{}}, w(ww) { }
    /// Constructor with coordinate and weight
    explicit weightedpt(const coord_t& x, weight_t ww = weight_t(1)): coord_t(x), w(ww) { }

    int i = 0;  ///< origin index
    weight_t w; ///< weight
};

/// PCA calculation
template<class _wpt>
class WPtsPCA {
public:
    /// associated weighted-point type
    typedef _wpt wpt_t;
    /// weight units shortcut
    typedef typename wpt_t::weight_t weight_t;
    /// component shortcut
    typedef typename wpt_t::component_t component_t;
    /// coordinate shortcut
    typedef typename wpt_t::coord_t coord_t;
    /// shortcut to dimensionality
    static constexpr int N = wpt_t::NDIM;

    // data
    coord_t mu{{}};                     ///< mean center
    array<array<double,N>,N> Cov{{}};   ///< covariance matrix
    array<array<double,N>,N> PCA{{}};   ///< orthogonal principal components vectors in PCA[i], largest to smallest
    array<double,N> width2{{}};         ///< spread along principal directions (eigenvalues of Cov), largest to smallest
    size_t n = 0;                       ///< number of points
    weight_t sw{};                      ///< sum of weights

    /// Default constructor
    WPtsPCA() { }

    /// Constructor, calculated from points
    explicit WPtsPCA(const vector<wpt_t>& v): n(v.size()) {
        if(!n) return;

        // weights vector
        TMatrixD w(1,n);
        for(size_t i=0; i<n; i++) { w(0,i) = v[i].w; }
        sw = weight_t(w.Sum());
        if(!sw) return;
        if(!(sw == sw)) throw std::runtime_error("Invalid NaN sum weights");

        // points matrix
        TMatrixD M(n,N);
        for(size_t i=0; i<n; i++)
            for(int j = 0; j<N; j++)
                M(i,j) = v[i][j];

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
    inline double sigma2(int a) const { return sw? width2[a]/sw : 0.; }
    /// rms spread along principal components direction
    inline double sigma(int a) const { return sqrt(sigma2(a)); }
    /// transverse width^2 from principal axis
    inline double wT2() const { double s = 0; for(int i = 1; i<N; ++i) s += width2[i]; return s; }
    /// transverse spread^2 from principal axis
    inline double sigmaT2() const { return sw? wT2()/sw : 0.; }
    /// transverse spread from principal axis
    inline double sigmaT() const { return sqrt(sigmaT2()); }
    /// coordinate along principal axis
    inline coord_t principal_coord(double u) const {
        auto x = mu;
        for(int i = 0; i<N; ++i) x[i] += PCA[0][i] * u;
        return x;
    }

    /// reverse direction
    void flip() {for(int i = 0; i<N-1; ++i) for(int j = 0; j<N; ++j) PCA[i][j] *= -1; }

    void operator+=(const WPtsPCA& P) {
        if(!sw) {
            *this = P;
            return;
        }

        WPtsPCA Pnew;

        Pnew.n = n + P.n;
        Pnew.sw = sw + P.sw;
        for(int j = 0; j<N; j++) Pnew.mu[j] = (mu[j]*sw + P.mu[j]*P.sw)/Pnew.sw;

        for(int i = 0; i<N; i++) {
            for(int j = 0; j<N; j++) {
                Pnew.Cov[i][j] = Cov[i][j] + P.Cov[i][j] + (mu[i]-P.mu[i])*(mu[j]-P.mu[j])*sw*P.sw/Pnew.sw;
            }
        }

        *this = Pnew;
        calcPrincipalComponents(TMatrixDSym(N, Cov[0].data()));
    }

    /// out-of-place sum
    const WPtsPCA operator+(const WPtsPCA& P) const { auto PP = *this; PP += P; return PP; }

    void calcPrincipalComponents(const TMatrixDSym& MCov) {
        const TMatrixDSymEigen eigen(MCov);
        const TVectorD& eigenVal = eigen.GetEigenValues();
        const TMatrixD& eigenVec = eigen.GetEigenVectors();
        for(int i = 0; i<N; i++) {
            width2[i] = eigenVal[i] > 0? eigenVal[i] : 0.; // fix negative values rounding error near 0
            for(int j = 0; j<N; j++) PCA[j][i] = eigenVec(i,j);
        }
    }

    void display() const {
        printf("Cloud of %zu points (total weight %g, average %g):\n", n, double(sw), n? sw/n : 0);
        if(N==3) {
            printf("\tmean = %g\t%g\t%g\t\tRMS = %g\t%g\t%g\n",
            double(mu[0]), double(mu[1]), double(mu[2]), sigma(0), sigma(1), sigma(2));
            for(int i=0; i<3; i++) {
                printf("\t%g\t%g\t%g\t\t%g\t%g\t%g\n",
                    Cov[i][0], Cov[i][1], Cov[i][2],
                PCA[i][0], PCA[i][1], PCA[i][2] );
            }
        }
    }
};

/// Shortcut type definition
template <size_t N, typename T = double, typename W = double>
using PointCloudPCA = WPtsPCA<weightedpt<N,T,W>>;

#endif
