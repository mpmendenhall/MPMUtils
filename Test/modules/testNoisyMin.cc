/// @file testNoisyMin.cc Test NoisyMin algorithm

#include "ConfigFactory.hh"
#include "GlobalArgs.hh"
#include "NoisyMin.hh"

#include <fstream>
using std::cout;

#include <TGraph.h>
#include <TPad.h>
#include <TRandom3.h>
#include <TCanvas.h>
#include <TPad.h>

#define NVAR 2

template<typename C>
void gEllipse(TGraph& g, const gsl_matrix* pca, const C& x0) {
    for(int k=0; k<=100; k++) {
        auto th = k*2*M_PI/100;
        auto c = cos(th);
        auto s = sin(th);
        g.SetPoint(k, x0[0] + c*gsl_matrix_get(pca,0,0) + s*gsl_matrix_get(pca,0,1),
                      x0[1] + c*gsl_matrix_get(pca,1,0) + s*gsl_matrix_get(pca,1,1));
    }
}

/// Plot out ellipse slice in specified axis plane from principal-axes matrix m
TGraph vEllipse(const gsl_matrix* m, double /*x0*/, double /*y0*/, size_t ax = 0, size_t ay = 1, int npts = 100) {
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

/// run fit step on points saved in file
void fitFile(const char* fname) {
    const size_t NDIM = 7;
    NoisyMin NM(NDIM);
    NM.verbose = 1;
    NM.x0 =             {0.32, -0.045, 12., 0.0028, 0.64, 1.6, 0.5};
    vector<double> dx = {.1,     .02,   2.,  .0002,  .05,  .2,  .15};
    NM.h = 0.1;
    for(size_t i=0; i<NDIM; i++) gsl_matrix_set(NM.dS, i, i, 3*dx[i]);
    NM.initRange();

    // load datapoints from file specified on command line
    std::ifstream f(fname);
    while(f.good()) {
        NM.fvals.emplace_back(NM.nDim());
        auto& v = NM.fvals.back();
        for(auto& c: v.x) f >> c;
        Quadratic::evalTerms(v.x, v.t);
        f >> v.f;
        f >> v.df2;
    }
    NM.fvals.pop_back();
    printf("Loaded %zu fit points\n", NM.fvals.size());
    NM.fitMinSingular();
}


REGISTER_EXECLET(testNoisyMin) {
    fitFile(requiredGlobalArg("f", "fit data file").c_str());
}


/*
int main(int, char**) {

    // noisy evaluation function
    //Quadratic<NVAR> Q(vector<double>({1.,2.,3.,4.,5.,6.,7.,8.,9.,10.}));
    Quadratic<NVAR> Q(vector<double>({1.,2.,3.,0.2,0.4,0.6}));
    TRandom3 TR;
    auto f = [&Q,&TR](NoisyMin<NVAR>::coord_t x) { return Q(x) + 0.2*TR.Gaus(); };

    // compare many random realizations
    size_t ntrials = 1000;
    TGraph g; // (2D plot) center fit points for each trial
    vector<TGraph> gE(4);           // error ellipse estimate from a few trials
    vector<TGraph> gS(gE.size());   // fit region from a few trials
    vector<NoisyMin<NVAR>::coord_t> vx0;

    for(size_t j = 0; j < ntrials; j++) {
        printf("\n\n---- trial %zu -----\n\n", j);

        // minimizer
        NoisyMin<NVAR> NM;
        NM.verbose = j < 10;
        // initial search range
        for(int i=0; i<NVAR; i++) gsl_matrix_set(NM.dS, i, i, 1.0);

        for(int u=0; u<5; u++) {
            for(int i=0; i<50; i++) NM.addSample(f);
            if(u%2) NM.fitMin();
            else NM.fitMinSingular();
        }
        g.SetPoint(j, NM.x0[0], NM.x0[1]);
        vx0.push_back(NM.x0);

        ////////////////////////////
        if(j >= gE.size()) continue;

        gE[j].SetMarkerColor(2+2*j);
        gE[j].SetLineColor(2+2*j);
        gEllipse(gE[j], NM.U_dx, NM.x0);

        gS[j].SetMarkerColor(2+2*j);
        gS[j].SetLineColor(2+2*j);
        gS[j].SetLineStyle(2);
        gEllipse(gS[j], NM.dS, NM.x0);
    }

    EigSymmWorkspace ESW(NVAR);
    auto A = gsl_matrix_calloc(NVAR,NVAR);
    auto l = gsl_vector_alloc(NVAR);
    sumSymm(A, l, vx0);
    ESW.decompSymm(A, l);
    for(size_t i=0; i<NVAR; i++) gsl_vector_set(l, i, sqrt(gsl_vector_get(l,i)/vx0.size()));
    rmul_diag(A, l);
    TGraph gf;
    gEllipse(gf, A, NoisyMin<NVAR>::coord_t{});

    TCanvas C;
    gPad->DrawFrame(-.06, -.06, .06, .06);

    //g.SetMarkerStyle(7);
    g.Draw("P");
    gf.Draw("L");
    for(auto& gg: gE) gg.Draw("L");
    for(auto& gg: gS) gg.Draw("L");
    //for(auto& gg: gR) gg.Draw("P");


    gPad->Print("searchpoints.pdf");

    return EXIT_SUCCESS;
}
*/
