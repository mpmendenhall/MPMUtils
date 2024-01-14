/// @file TLS_Solver.hh Total Least Squares (TLS) solver for line through point cloud
// This file was produced under the employ of the United States Government,
// and is consequently in the PUBLIC DOMAIN, free from all provisions of
// US Copyright Law (per USC Title 17, Section 105).
//
// -- Michael P. Mendenhall, 2015

#ifndef TLS_SOLVER_HH
#define TLS_SOLVER_HH

#include "LAPACKE_Matrix.hh"
#include "VarMat.hh"
#include <stdlib.h> // for size_t

/// Total Least Squares (TLS) solver for line through point cloud
class TLS_Solver {
public:
    /// Constructor
    explicit TLS_Solver(size_t nn, size_t mm = 0):
    n(nn), B(mm,n), mu(n), v(n), mySVD(nullptr) { }
    /// Destructor
    ~TLS_Solver() { delete mySVD; }
    /// solve TLS system
    void solve();
    /// Get sum of squares of residuals
    double getSSR() const;

    const size_t n;             ///< number of dimensions

    VarMat<double> B;           ///< data points
    VarVec<double> mu;          ///< mean center
    VarVec<double> v;           ///< direction vector

protected:
    LAPACKE_Matrix_SVD<double,double>* mySVD;
};

#endif
