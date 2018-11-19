/// \file "NoisyMin.hh" Minimizer for ``noisy'' function evaluation

#ifndef NOISYMIN_HH
#define NOISYMIN_HH

#include "Quadratic.hh"
#include "GeomCalcUtils.hh"
#include "LinMin.hh"
#include <Math/QuasiRandom.h>
#include <gsl/gsl_cblas.h>
#include <gsl/gsl_eigen.h>
#include <vector>
using std::vector;

/// Minimizer for N-dimensional ``noisy'' function evaluation
template<size_t N, typename T = double>
class NoisyMin {
public:
    /// Number of terms in quadratic surface fit
    static constexpr int NTERMS = Quadratic<N,T>::NTERMS;

    /// Constructor
    NoisyMin(): dQ(gsl_matrix_calloc(N,N)), v1(gsl_vector_alloc(N)), v2(gsl_vector_alloc(N)), QRNG(N) { }
    /// Destructor
    ~NoisyMin() {
        gsl_matrix_free(dQ); gsl_vector_free(v1); gsl_vector_free(v2);
    }

    /// coordinate in minimization space
    typedef array<T,N> coord_t;

    /// minimizer function evaluation data at a point
    struct evalpt {
        evalpt(const coord_t& xx): x(xx) { }
        coord_t x;      ///< position
        typename Quadratic<N,T>::terms_t t;  ///< fitter terms at x
        T f = 0;        ///< function evaluated at position
        T df2 = 1;      ///< estimated uncertainty^2
    };

    Quadratic<N,T> Q;       ///< estimated quadratic minimum surface
    gsl_matrix* dQ;         ///< uncertainty ellipse on Q minimum (1-sigma orthogonal vectors)
    LinMin LM;              ///< linear minimizer

    /// add evaluated point
    template<typename F>
    evalpt& addSample(F& f) {
        fvals.emplace_back(nextSample());
        auto& p = fvals.back();
        p.f = f(p.x);
        Quadratic<N,T>::evalTerms(p.x, p.t);
        return p;
    }

    /// request next sampling point
    coord_t nextSample(T nsigma = 1) {
        coord_t r;
        do {
            QRNG.Next(r.data());
            for(auto& x: r) x = 2*x-1;
        } while(vmag2(r) > 1.);

        std::copy(r.begin(), r.end(), v1->data);
        std::copy(QC.x0.begin(), QC.x0.end(), v2->data);
        // v2 = x0 + dQ * r
        gsl_blas_dgemv(CblasNoTrans, nsigma, dQ, v1, 1., v2);

        coord_t x;
        std::copy(v2->data, v2->data+N, x.begin());
        return x;
    }

    /// update estimated minimum
    void fitMin() {
        LM.resize(fvals.size(), NTERMS);
        vector<T> y(fvals.size());
        //auto& x0 = Q.factor(); // for weighting by distance to center
        size_t i = 0;
        for(auto& p: fvals) {
            int j=0;
            for(auto x: p.t) LM.setM(i, j++, x);
            y[i] = p.f;
            i++;
        }
        LM.solve(y);
        LM.getx(y);
        Q = Quadratic<N,T>(y);
        QC.decompose(Q);
    }

protected:
    QuadraticCholesky<N,T> QC;      ///< quadratic decomposition helper
    gsl_vector *v1, *v2;            ///< temporary N calcultion vectors
    gsl_vector* vNt;                ///< temporary NTERMS calculation vector
    vector<evalpt> fvals;           ///< collected function evaluations
    ROOT::Math::QuasiRandomNiederreiter QRNG;   ///< quasirandom distribution generator
};

#endif
