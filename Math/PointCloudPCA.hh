/// \file PointCloudPCA.hh Principal Components Analysis for weighted point cloud
// Michael P. Mendenhall, LLNL 2016

#ifndef POINTCLOUDPCA_HH
#define POINTCLOUDPCA_HH

#include <vector>
using std::vector;
#include <cstddef> // for size_t
#include <cmath>

/// point with weight in point cloud
struct weightedpt {
    /// Constructor
    weightedpt(double xx=0, double y=0, double z=0, double ww=1): x{xx,y,z}, w(ww) { }
    double x[3];    ///< position
    double w = 1;   ///< weight
    /// display to stdout
    void display() const;
};

/// summary statistics calculator
class PointCloudPCA {
public:
    /// Constructor, calculated from points
    PointCloudPCA(const vector<weightedpt>& v = {});
    /// display to stdout
    void display() const;
    /// rm spread squared along principal components direction
    inline double sigma2(int a) const { return width2[a]/sw; }
    /// rms spread along principal components direction
    inline double sigma(int a) const { return sqrt(sigma2(a)); }
    /// transverse width^2 from principal axis
    inline double wT2() const { return width2[1] + width2[2]; }
    /// transverse spread from principal axis
    inline double sigmaT2() const { return wT2()/sw; }

    /// Combine data from another group of points
    void operator+=(const PointCloudPCA& P);
    /// combined sum
    const PointCloudPCA operator+(const PointCloudPCA& P) const { auto PP = *this; PP += P; return PP; }

    /// Recalculate from updated covariance matrix
    void recalc();
    /// flip sign of components
    void flip();

    double mu[3];       ///< mean center
    double Cov[3][3];   ///< covariance matrix
    double PCA[3][3];   ///< orthogonal principal components vectors in PCA[i], largest to smallest
    double width2[3];   ///< spread along principal directions (eigenvalues of Cov), largest to smallest
    size_t n = 0;       ///< number of points
    double sw = 0;      ///< sum of weights
};

#endif
