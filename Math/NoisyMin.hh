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
template<size_t N, typename T = double>
class NoisyMin {
public:
    /// Number of terms in quadratic surface fit
    static constexpr int NTERMS = Quadratic<N,T>::NTERMS;

    /// Constructor
    NoisyMin(): LM(NTERMS), dQ0(gsl_matrix_calloc(N,N)), dS(gsl_matrix_calloc(N,N)),
    v1(gsl_vector_alloc(N)), v2(gsl_vector_alloc(N)), QRNG(N), EWS(N) { }
    /// Destructor
    ~NoisyMin() {
        for(auto m: {dQ0, dS}) gsl_matrix_free(m);
        for(auto v: {v1, v2}) gsl_vector_free(v);
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

    Quadratic<N,T> Q;               ///< estimated quadratic minimum surface
    QuadraticCholesky<N,T> QChol;   ///< Cholesky decomposition of Q
    coord_t x0 = {};        ///< estimated minimum location
    LinMin LM;              ///< linear minimizer (solves for coefficients in Q)
    gsl_matrix* dQ0;        ///< fit stats uncertainty ellipse on Q minimum (1-sigma orthogonal vector columns)
    gsl_matrix* dS;         ///< search/sampling region principal axes columns for minimum in parameter space
    double h = 1;           ///< height of ``1 sigma'' minima search region
    int verbose = 0;        ///< debugging verbosity level
    size_t step = 0;        ///< fit step number
    double nSigmaStat = 4;  ///< statistical uncertainty search region expansion

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
        std::copy(x0.begin(), x0.end(), v2->data);
        // v2 = x0 + dS * r
        gsl_blas_dgemv(CblasNoTrans, nsigma, dS, v1, 1., v2);

        coord_t x;
        for(size_t i=0; i<N; i++) x[i] = gsl_vector_get(v2,i);
        return x;
    }

    /// update estimated minimum
    void fitMin() {
        // filter points to search region (except on first step)
        vector<evalpt> vs;
        if(!(step++)) vs = fvals;
        else {
            for(auto& p: fvals) {
                for(size_t i=0; i<N; i++) gsl_vector_set(v1, i, p.x[i] - x0[i]);
                gsl_blas_dtrmv(CblasLower, CblasTrans, CblasNonUnit, sE.EC.L, v1);
                if(gsl_blas_dnrm2(v1) < 1) vs.push_back(p);
            }
        }
        if(verbose) printf("\n**** Fit step %zu to %zu/%zu datapoints...\n", step, vs.size(), fvals.size());

        // fit surface around minimum
        LM.setNeq(vs.size());
        vector<T> y(vs.size());
        size_t i = 0;
        for(auto& p: vs) {
            int j = 0;
            for(auto x: p.t) LM.setM(i, j++, x);
            y[i] = p.f;
            i++;
        }
        LM.solve(y);
        LM.getx(y);
        Q = Quadratic<N,T>(y);  // estimated minimum surface
        QChol.decompose(Q);     // find minimum position (and Cholesky form)
        x0 = QChol.x0;
        for(i=0; i<N; i++) for(size_t j=0; j<=i; j++) gsl_matrix_set(sE.E1.L, i, j, gsl_matrix_get(QChol.L, i, j)/sqrt(h));

        // fit parameter uncertainties to minimum location uncertainty
        auto P = LM.calcPCA();
        auto l = LM.PCAlambda();
        auto s2 = LM.ssresid()/LM.nDF();
        if(verbose) printf("RMS deviation %g\n", sqrt(s2));
        gsl_matrix_set_zero(dQ0);
        for(i=0; i<NTERMS; i++) {
            auto yy = y;
            for(size_t j=0; j<NTERMS; j++) yy[j] += gsl_matrix_get(P,j,i)*sqrt(gsl_vector_get(l,j)*s2);
            Quadratic<N,T> Qi(yy);
            QC.decompose(Qi);
            for(size_t j=0; j<N; j++) gsl_vector_set(v1, j, QC.x0[j] - x0[j]);
            gsl_blas_dsyr(CblasLower, 1., v1, dQ0);
        }

        // n-sigma stats uncertainty inverse Cholesky form
        for(i=0; i<N; i++) for(size_t j=0; j<=i; j++) gsl_matrix_set(sE.E2.L, i, j, gsl_matrix_get(dQ0, i, j)*nSigmaStat*nSigmaStat);
        gsl_linalg_cholesky_decomp1(sE.E2.L);
        gsl_linalg_cholesky_invert(sE.E2.L);
        gsl_linalg_cholesky_decomp1(sE.E2.L);

        // stats uncertainty 1*sigma principal axes
        EWS.decompSymm(dQ0, v1);
        for(i=0; i<N; i++) gsl_vector_set(v1, i, sqrt(gsl_vector_get(v1,i)));
        rmul_diag(dQ0, v1);
        if(verbose > 1) { printf("dQ0:\n"); displayM(dQ0); }

        // update search region
        sE.calcCovering();              // ellipse containing both
        Quadratic<N,T> Qs;              // A-form ellipse representation
        sE.EC.fillA(Qs);                // Cholesky to A
        QP.decompose(Qs);               // search region principal axes
        gsl_matrix_memcpy(dS, QP.USi);  // store to search region dS

        if(verbose) display();
    }

    void display() {
        printf("NoisyMin fitter of %zu parameters wth %zu datapoints\n", N, fvals.size());
        for(size_t i=0; i<N; i++) {
            vector<T> v(N);
            v[i] = 1.0;
            printf("[%zu]\t%g\t~%g (dh)\t~%g (stat)\n", i, x0[i], QChol.projLength(v)*sqrt(h), sE.E2.projLength(v)/nSigmaStat);
        }
    }

protected:
    QuadraticCholesky<N,T> QC;      ///< quadratic decomposition helper
    CoveringEllipse<N,T> sE;        ///< search region around minima, from surface + stat. uncertainty
    QuadraticPCA<N,T> QP;           ///< principal axes decomposition helper
    gsl_vector *v1, *v2;            ///< temporary N calcultion vectors
    gsl_vector* vNt;                ///< temporary NTERMS calculation vector
    gsl_matrix* Mnn;                ///< temporary
    vector<evalpt> fvals;           ///< collected function evaluations
    ROOT::Math::QuasiRandomNiederreiter QRNG;   ///< quasirandom distribution generator
    EigSymmWorkspace EWS;           ///< NxN eigendecomposition workspace
};

#endif
