/// \file "NoisyMin.hh" Minimizer for ``noisy'' function evaluation

#ifndef NOISYMIN_HH
#define NOISYMIN_HH

#include "LinalgHelpers.hh"
#include "Quadratic.hh"
#include "GeomCalcUtils.hh"
#include "LinMin.hh"
#include <Math/QuasiRandom.h>
#include <gsl/gsl_cblas.h>
#include <gsl/gsl_eigen.h>
#include <vector>
using std::vector;

/// Minimizer for N-dimensional ``noisy'' function evaluation
class NoisyMin {
public:

    /// shorthand notation
    typedef vector<double> vec_t;
    const size_t N;         ///< Number of dimensions being minimized over
    const size_t NTERMS;    ///< Number of terms in quadratic surface fit

    /// Constructor
    NoisyMin(size_t n): N(n), NTERMS(Quadratic::nterms(N)), x0(N), Mt(NTERMS) {
        for(auto& m: Mt) m = gsl_matrix_alloc(N,N);
    }
    /// Destructor
    ~NoisyMin();

    /// evaluated datapoint for fit
    struct evalpt {
        /// constructor with dimensions
        evalpt(size_t n): x(n), t(Quadratic::nterms(n)) { }
        vec_t x;        ///< position
        vec_t t;        ///< fitter terms at x
        double f = 0;   ///< function evaluated at position
        double df2 = 1; ///< estimated uncertainty^2
    };

    /// add evaluated point from supplied function
    template<typename F>
    evalpt& addSample(F& f) {
        fvals.emplace_back(N);
        auto& p = fvals.back();
        p.x = nextSample();
        p.f = f(p.x);
        Quadratic::evalTerms(p.x, p.t);
        return p;
    }

    /// request next sampling point
    vec_t nextSample(double nsigma = 1);
    /// perform fit update step (non-singular solutions)
    void fitMin();
    /// perform fit update step (non-positive-definite Hessian)
    void fitMinSingular();
    /// show summary information
    void display();
    /// fit Q to points in current region
    void fitHessian();

    QuadraticCholesky SR0{N};                   ///< initial search range/limits ellipse
    gsl_matrix* dS = gsl_matrix_alloc(N,N);     ///< fit/sampling region (principal axes columns)

    vector<evalpt> fvals;                       ///< collected function evaluations

    LinMin LM{NTERMS};                          ///< fitter for terms in Q
    Quadratic Q{N};                             ///< estimated quadratic minimum surface
    QuadraticCholesky QChol{N};                 ///< Cholesky decomposition of Q
    gsl_matrix* U_q = gsl_matrix_alloc(N,N);    ///< Unitary SVD (columns) of Q: A = U_q S_q U_q^T
    gsl_vector* S_q = gsl_vector_alloc(N);      ///< eigenvalues ("sigma^2") diagonal for Uq
    gsl_vector* dS_q = gsl_vector_alloc(N);     ///< fit uncertainties on S_q

    vec_t x0;                                   ///< current best fit estimate
    gsl_matrix* Udx0 = gsl_matrix_alloc(N,N);   ///< Unitary principal axes (columns) of uncertainty ellipse on x0
    gsl_vector* Sdx0 = gsl_vector_alloc(N);     ///< eigenvalues for Udx0

    double h = 1;                               ///< height of ``1 sigma'' minima search region
    int verbose = 0;                            ///< debugging verbosity level
    double nSigmaStat = 4;                      ///< statistical uncertainty search region expansion

    /// generate variations according to LM covariance
    vector<Quadratic> LMvariants();

protected:
    QuadraticCholesky QC{N};                    ///< quadratic decomposition helper
    EigSymmWorkspace EWS{N};                    ///< NxN eigendecomposition workspace
    CoveringEllipse sE{N};                      ///< search region around minima, from surface + stat. uncertainty
    QuadraticPCA QP{N};                         ///< principal axes decomposition helper

    gsl_matrix* M1 = gsl_matrix_alloc(N,N);     ///< temporary calculation matrix
    vector<gsl_matrix*> Mt;                     ///< temporary calculation matrices
    gsl_vector* v1 = gsl_vector_alloc(N);       ///< temporary calculation vector
    gsl_vector* v2 = gsl_vector_alloc(N);       ///< temporary calculation vector

    ROOT::Math::QuasiRandomNiederreiter QRNG{(unsigned int)N};   ///< quasirandom distribution generator
};

#endif
