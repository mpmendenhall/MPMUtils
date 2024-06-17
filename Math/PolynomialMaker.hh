/// @file PolynomialMaker.hh Solve for polynomial meeting conditions

#include "LinalgHelpers.hh"
#include <array>
using std::array;

template<size_t N>
class PolynomialCoeffs: public array<double, N> {
public:
    /// evaluate polynomial
    double operator()(double x) const {
        double s = 0;
        double xi = 1;
        for(auto c: *this) {
            s += c * xi;
            xi *= x;
        }
        return s;
    }

    /// evaluate derivative
    double derivative(double x) const {
        if(N < 2) return 0;
        double s = (*this)[1];
        double xi = x;
        for(size_t i = 2; i < N; ++i) {
            s += i * (*this)[i] * xi;
            xi *= x;
        }
        return s;
    }
};

template<size_t N>
class PolynomialMaker: public PolynomialCoeffs<N> {
public:
    /// Constructor
    PolynomialMaker(): P(gsl_permutation_alloc(N)) { }
    /// Destructor
    ~PolynomialMaker() { gsl_permutation_free(P); }

    /// Set value constraint at x
    void constrain_value_at(size_t i, double x) {
        double xi = 1;
        for(size_t j = 0; j < N; ++j) {
            M(i, j) = xi;
            xi *= x;
        }
    }

    /// Set derivative constraint at x
    void constrain_derivative_at(size_t i, double x) {
        M(i, 0) = 0;
        double xi = 1;
        for(size_t j = 1; j < N; ++j) {
            M(i, j) = j*xi;
            xi *= x;
        }
    }

    /// Fix constraints matrix (and QR decompose, modifying M)
    void finalize_constraints() {
        int signum = 0;
        gsl_linalg_LU_decomp(M, P, &signum);
    }

    /// set constraint value RHS vector
    void set_constraint_rhs(size_t i, double y) { v(i) = y; }

    /// Solve constraints for coefficients
    void solve() {
        gsl_linalg_LU_solve(M, P, v, X);
        for(size_t i=0; i<N; ++i) (*this)[i] = X(i);
    }

protected:
    gsl_matrix_wrapper M{N,N};  ///< constraints matrix (-> LU)
    gsl_vector_wrapper v{N};    ///< RHS vector
    gsl_permutation* P;         ///< LU decomposition "P"
    gsl_vector_wrapper X{N};    ///< solution vector
};
