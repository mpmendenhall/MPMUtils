#ifndef TLS_SOLVER_HH
#define TLS_SOLVER_HH

#include "LAPACKE_Matrix.hh"

/// Total Least Squares (TLS) solver for line through point cloud
class TLS_Solver {
public:
    /// Constructor
    TLS_Solver(size_t mm, size_t nn);
    /// Destructor
    ~TLS_Solver();
    /// solve TLS system
    void solve();
    
    const size_t m;             ///< number of points
    const size_t n;             ///< number of dimensions
    
    VarMat<double> B;           ///< data points
    VarVec<double> mu;          ///< mean center
    VarVec<double> v;           ///< direction vector
    
protected:
    LAPACKE_Matrix_SVD<double,double>* mySVD = NULL;
};

#endif
