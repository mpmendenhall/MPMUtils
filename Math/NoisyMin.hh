/// \file "NoisyMin.hh" Minimizer for ``noisy'' function evaluation

#ifndef NOISYMIN_HH
#define NOISYMIN_HH

#include "LinalgHelpers.hh"
#include "Quadratic.hh"
#include "LinMin.hh"
#include <Math/QuasiRandom.h>
#include <iostream>
#include <vector>
using std::vector;
#include <string>

/// Minimizer for N-dimensional ``noisy'' function evaluation
/**
    - Set initial search range in dS, initial guess in x0
    - call initRange() to set sampling range bounds SR0 from dS, x0
    - repeat to desired convergence:
        - add points using addSample(f) on evaluated function
        - call fitMinSingular() for an update step
*/
class NoisyMin {
protected:
    size_t N;                       ///< Number of dimensions being minimized over
    size_t NTERMS;                  ///< Number of terms in quadratic surface fit
public:
    typedef vector<double> vec_t;   ///< convenience shorthand

    /// evaluated datapoint for fit
    struct evalpt {
        /// constructor with dimensions
        evalpt(size_t n): x(n), t(Quadratic::nterms(n)) { }
        vec_t x;        ///< position
        vec_t t;        ///< fitter terms at x
        double f = 0;   ///< function evaluated at position
        double df2 = 1; ///< estimated uncertainty^2
    };

    /// Constructor, with number of dimensions
    NoisyMin(size_t n);

    /// get number of terms
    size_t nTerms() const { return NTERMS; }
    /// get number of dimensions
    size_t nDim() const { return N; }
    /// initialize search range from dS, x0
    void initRange();
    /// call after initRange (and modifying SRm) to set minimum step limits
    void initMinStep();

    /// add evaluated point from supplied function
    template<typename F>
    evalpt& addSample(F& f);
    /// perform fit update step (non-singular solutions)
    void fitMin();
    /// perform fit update step (non-positive-definite Hessian)
    void fitMinSingular();
    /// show summary information
    void display();
    /// show search range info
    void displaySearchRange() const;

    // initial values
    vec_t x0;                       ///< current best fit estimate
    gsl_matrix_wrapper dS{N,N};     ///< fit/sampling region (principal axis columns)

    // result statistical uncertainties
    gsl_matrix_wrapper U_dx{N,N};   ///< Unitary principal axes (columns) of uncertainty ellipse
    gsl_vector_wrapper S_dx{N};     ///< eigenvalues 1/sigma^2 for U_dx
    // result Hessian
    gsl_matrix_wrapper U_q{N,N};    ///< Unitary SVD (columns) A = U_q S_q U_q^T
    gsl_vector_wrapper S_q{N};      ///< eigenvalues ("1/sigma^2") diagonal for U_q

    double h = 1;                   ///< height of ``1 sigma'' minima search region
    int verbose = 0;                ///< debugging verbosity level
    double nSigmaStat = 4;          ///< statistical uncertainty search region expansion

    // internal / debugging quantities
    vector<std::string> vnames{N};  ///< variable names
    vector<evalpt> fvals;           ///< collected function evaluations
    gsl_matrix_wrapper SR0{N,N};    ///< initial search range/limits ellipse (Cholesky form)
    bool minStep = false;           ///< whether to apply minimum step limits
    gsl_matrix_wrapper SRm{N,N};    ///< minimum search range ellipse (start in PCA form; converted to Cholesky form)

    LinMin LM{NTERMS};              ///< fitter for quadratic surface x^T A x + b^T x + c around minimum
    double k0 = 0;                  ///< fit minimum value
    double dk2 = 0;                 ///< statistical uncertainty^2 on k0

    /// request next sampling point location
    vec_t nextSample(double nsigma = 1);
    /// generate variations according to LM covariance
    vector<Quadratic> LMvariants();
    /// fit LM to points in current region; return convenience quadratic
    Quadratic fitHessian();

protected:
    /// update search range assuming sE.E1.L, sE.E2.L in Cholesky form
    void updateRange();

    QuadraticCholesky QC{N};    ///< quadratic decomposition helper
    EigSymmWorkspace EWS{N};    ///< NxN eigendecomposition workspace
    CoveringEllipse sE{N};      ///< search region around minima, from surface + stat. uncertainty
    QuadraticPCA QP{N};         ///< principal axes decomposition helper

    gsl_matrix_wrapper M1{N,N}; ///< temporary calculation matrix
    gsl_matrix_wrapper M2{N,N}; ///< temporary calculation matrix
    gsl_vector_wrapper v1{N};   ///< temporary calculation vector
    gsl_vector_wrapper v2{N};   ///< temporary calculation vector

    ROOT::Math::QuasiRandomNiederreiter QRNG{(unsigned int)N};   ///< quasirandom distribution generator
    size_t QRNGn = 0;           ///< number of points pulled from QRNG

    friend std::ostream& operator<< (std::ostream &o, const NoisyMin& NM);
    friend std::istream& operator>> (std::istream &i, NoisyMin& NM);
};

/// serialization
std::ostream& operator<< (std::ostream &o, const NoisyMin& NM);
/// deserialization
std::istream& operator>> (std::istream &i, NoisyMin& NM);

///////////////////////
///////////////////////

template<typename F>
NoisyMin::evalpt& NoisyMin::addSample(F& f) {
    fvals.emplace_back(N);
    auto& p = fvals.back();
    p.x = nextSample();
    p.f = f(p.x);
    Quadratic::evalTerms(p.x, p.t);
    return p;
}

/// output evalpt to text line
std::ostream& operator<<(std::ostream& o, const NoisyMin::evalpt& p);
/// deserialize (requires correct dimension to already be set)
std::istream& operator>>(std::istream& i, NoisyMin::evalpt& p);

#endif
