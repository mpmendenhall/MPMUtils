/// \file testNoisyMin.cc Test NoisyMin algorithm

#include "CodeVersion.hh"
#include "NoisyMin.hh"
#include <iostream>
#include <TGraph.h>
#include <TPad.h>
#include <TRandom3.h>

template<typename T, size_t N>
std::ostream& operator<<(std::ostream& o, const array<T,N>& v) {
    o << "< ";
    for(auto& x: v) o << x << " ";
    o << ">";
    return o;
}

#define NVAR 2

/// Plot out ellipse slice in specified axis plane from principal-axes matrix m
TGraph vEllipse(const gsl_matrix* m, double x0, double y0, size_t ax = 0, size_t ay = 1, int npts = 100) {
    TGraph g(npts+1);

    auto v = gsl_vector_calloc(m->size1);
    auto v2 = gsl_vector_alloc(m->size1);

    for(int k=0; k<=npts; k++) {
        auto th = k*2*M_PI/npts;
        auto c = cos(th);
        auto s = sin(th);
        gsl_vector_set(v, ax, c);
        gsl_vector_set(v, ay, s);
        gsl_blas_dgemv(CblasNoTrans, 1., m, v, 0., v2);
        g.SetPoint(k, gsl_vector_get(v2, ax), gsl_vector_get(v2, ay));
    }

    gsl_vector_free(v);
    gsl_vector_free(v2);
    return g;
}

int main(int, char**) {
    CodeVersion::display_code_version();

    // noisy evaluation function
    //Quadratic<NVAR> Q(vector<double>({1.,2.,3.,4.,5.,6.,7.,8.,9.,10.}));
    Quadratic<NVAR> Q(vector<double>({1.,2.,3.,0,0,0}));
    TRandom3 TR;
    auto f = [&Q,&TR](NoisyMin<NVAR>::coord_t x) { return Q(x) + 0.1*TR.Gaus(); };

    // compare many random realizations
    size_t ntrials = 10;
    TGraph g; // (2D plot) center fit points for each trial
    vector<TGraph> gR(2);           // realizations from a few trials
    vector<TGraph> gE(gR.size());   // error ellipse estimate from a few trials

    for(size_t j = 0; j < ntrials; j++) {
        printf("\n\n---- trial %zu -----\n\n", j);

        // minimizer
        NoisyMin<NVAR> NM;
        // initial search range
        //NM.x0 = {-1.8, -0.25};
        for(int i=0; i<NVAR; i++) gsl_matrix_set(NM.dS, i, i, 2.0);

        for(int u=0; u<4; u++) {
            for(int i=0; i<50; i++) NM.addSample(f);
            NM.fitMin();
        }

        g.SetPoint(j, NM.x0[0], NM.x0[1]);

        continue;

        ////////////////////////////
        if(j >= gR.size()) continue;

/*
        gR[j].SetMarkerColor(2+2*j);

        auto s = sqrt(NM.LM.ssresid());
        //auto Mp0 = gsl_matrix_calloc(1000, NVAR);
        //printf("RMS sigma = %g\n", s/sqrt(NM.LM.nEq()-NM.LM.nVar()));

        for(int k = 0; k < 1000; k++) {
            // unit N-gaussian sphere
            vector<double> rg(NM.LM.nVar());
            for(auto& x: rg) x = TR.Gaus()*s;
            // corresponding realization
            vector<double> r;
            NM.LM.getRealization(rg, r);
            Quadratic<NVAR> Qr(r);
            QC.decompose(Qr);
            gR[j].SetPoint(k, QC.x0[0], QC.x0[1]);

            //auto x1 = Qr.factor();
            //for(int a=0; a < NVAR; a++) gsl_matrix_set(Mp0, k, a, x1[a]-x0[a]);
        }

        // parameter uncertainties transformed into x0 uncertainties
        auto Mp = gsl_matrix_calloc(NM.LM.nVar(), NVAR);
        for(int k = 0; k < NM.LM.nVar(); k++) {
            vector<double> rg(NM.LM.nVar());
            rg[k] = s;
            NM.LM.getRealization(rg, r);

            Quadratic<NVAR> Qr(r);
            auto& x1 = Qr.factor();
            for(int l=0; l<NVAR; l++) gsl_matrix_set(Mp, k, l, x1[l]-x0[l]);
        }

        auto MM = gsl_matrix_calloc(NVAR, NVAR);
        //if(gsl_blas_dsyrk(CblasLower, CblasTrans, 1./1000, Mp0, 0., MM)) throw;
        if(gsl_blas_dsyrk(CblasLower, CblasTrans, 1., Mp, 0., MM)) throw;
        auto esw = gsl_eigen_symmv_alloc(NVAR);
        auto PCA = gsl_matrix_alloc(NVAR, NVAR);
        auto lPCA = gsl_vector_alloc(NVAR);
        if(gsl_eigen_symmv(MM, lPCA, PCA, esw)) throw;

        gE[j].SetMarkerColor(2+2*j);
        gE[j].SetLineColor(2+2*j);
        gE[j].SetMarkerStyle(7);

        auto s0 = sqrt(gsl_vector_get(lPCA,0));
        auto s1 = sqrt(gsl_vector_get(lPCA,1));
        for(int k=0; k<=100; k++) {
            auto th = k*2*M_PI/100;
            auto c = cos(th);
            auto s = sin(th);

            gE[j].SetPoint(k, x0[0] + c*gsl_matrix_get(PCA,0,0)*s0 + s*gsl_matrix_get(PCA,0,1)*s1,
                              x0[1] + c*gsl_matrix_get(PCA,1,0)*s0 + s*gsl_matrix_get(PCA,1,1)*s1);
        }
        gE[j].SetPoint(101, x0[0], x0[1]);
        */
    }

    //g.SetMarkerStyle(7);
    g.Draw("AP");
    for(auto& gg: gR) gg.Draw("P");
   // for(auto& gg: gE) gg.Draw("L");

    gPad->Print("searchpoints.pdf");

    return EXIT_SUCCESS;
}
