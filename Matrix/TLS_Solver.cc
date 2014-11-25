#include "TLS_Solver.hh"

TLS_Solver::TLS_Solver(size_t mm, size_t nn): m(mm), n(nn), B(m,n), mu(n), v(n) {
}

TLS_Solver::~TLS_Solver() {
    if(mySVD) delete mySVD;
}

void TLS_Solver::solve() {
    // mean value
    mu = B.getColSum()/m;
    for(size_t mm = 0; mm < m; mm++)
        for(size_t nn = 0; nn < n; nn++)
            B(mm,nn) -= mu[nn];
            
    // SVD
    if(mySVD) delete mySVD;
    mySVD = new LAPACKE_Matrix_SVD<double,double>(B);
    v = mySVD->getRightSVec(0);
}

double TLS_Solver::getSSR() const {
    return (B*v).mag2();
}

