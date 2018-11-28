/// \file NoisyMin.cc

#include "NoisyMin.hh"
#include <gsl/gsl_cblas.h>
#include <gsl/gsl_eigen.h>
#include "GeomCalcUtils.hh"

NoisyMin::NoisyMin(size_t n): N(n),
NTERMS(Quadratic::nterms(N)), x0(N) {
    gsl_matrix_set_identity(dS);
}

NoisyMin::~NoisyMin() {
    for(auto m: {U_dx, dS, U_q, M1, M2}) gsl_matrix_free(m);
    for(auto v: {S_dx, S_q, v1, v2}) gsl_vector_free(v);
}

void NoisyMin::initRange() {
    sE.EC.x0 = SR0.x0 = x0;
    gsl_matrix_memcpy(sE.EC.L, dS);
    invert_colnorms(sE.EC.L);
    gsl_blas_dsyrk(CblasLower, CblasTrans, 1.0, sE.EC.L, 0., SR0.L);
    gsl_linalg_cholesky_decomp(SR0.L);
    gsl_matrix_memcpy(sE.EC.L, SR0.L);

    if(verbose) { printf("Initializing search range:\n"); displayM(dS); }
}

NoisyMin::vec_t NoisyMin::nextSample(double nsigma) {
    vec_t r(N);
    do { // TODO more elegant algorithm!
        QRNG.Next(r.data());
        for(auto& x: r) x = 2*x-1;
    } while(vmag2(r) > 1.);

    std::copy(r.begin(), r.end(), v1->data);
    std::copy(x0.begin(), x0.end(), v2->data);
    // v2 = x0 + dS * r
    gsl_blas_dgemv(CblasNoTrans, nsigma, dS, v1, 1., v2);

    vec_t x(N);
    for(size_t i=0; i<N; i++) x[i] = gsl_vector_get(v2,i);
    return x;
}

Quadratic NoisyMin::fitHessian() {
    // filter points to search region
    vector<evalpt> vs;
    for(auto& p: fvals) {
        for(size_t i=0; i<N; i++) gsl_vector_set(v1, i, p.x[i] - x0[i]);
        gsl_blas_dtrmv(CblasLower, CblasTrans, CblasNonUnit, sE.EC.L, v1);
        if(gsl_blas_dnrm2(v1) < 1) vs.push_back(p);
    }
    if(verbose) printf("\n**** NoisyMin fitting %zu/%zu datapoints...\n", vs.size(), fvals.size());

    // fit surface around minimum
    LM.setNeq(vs.size());
    vec_t y(vs.size());
    size_t i = 0;
    for(auto& p: vs) {
        int j = 0;
        for(auto x: p.t) LM.setM(i, j++, x);
        y[i] = p.f;
        i++;
    }
    LM.solve(y);
    LM.getx(y);
    Quadratic Q(N);
    Q.setCoeffs(y);  // estimated minimum surface
    if(verbose) Q.display();
    return Q;
}

vector<Quadratic> NoisyMin::LMvariants() {
    auto P = LM.calcPCA();
    auto l = LM.PCAlambda();
    auto s2 = LM.ssresid()/LM.nDF();
    if(verbose) printf("RMS deviation %g\n", sqrt(s2));
    vec_t y;
    LM.getx(y);
    vector<Quadratic> vQ;
    for(size_t i=0; i<NTERMS; i++) {
        auto yy = y;
        for(size_t j=0; j<NTERMS; j++) yy[j] += gsl_matrix_get(P,j,i)*sqrt(gsl_vector_get(l,i)*s2);
        vQ.emplace_back(N);
        vQ.back().setCoeffs(yy);
    }
    return vQ;
}

void NoisyMin::fitMin() {
    auto Q = fitHessian();  // update fit
    QC.decompose(Q);     // find minimum position (and Cholesky form)
    if(verbose) QC.display();
    x0 = QC.x0;
    for(size_t i=0; i<N; i++) for(size_t j=0; j<=i; j++) gsl_matrix_set(sE.E1.L, i, j, gsl_matrix_get(QC.L, i, j)/sqrt(h));

    // fit parameter uncertainties to minimum location uncertainty
    auto vQ = LMvariants();
    for(size_t i=0; i<NTERMS; i++) {
        QC.decompose(vQ[i]);
        for(size_t j=0; j<N; j++) gsl_vector_set(v1, j, QC.x0[j] - x0[j]);
        gsl_blas_dsyr(CblasLower, 1., v1, U_dx);
    }

    // n-sigma stats uncertainty inverse Cholesky form
    for(size_t i=0; i<N; i++) for(size_t j=0; j<=i; j++) gsl_matrix_set(sE.E2.L, i, j, gsl_matrix_get(U_dx, i, j)*nSigmaStat*nSigmaStat);
    gsl_linalg_cholesky_decomp(sE.E2.L);
    gsl_linalg_cholesky_invert(sE.E2.L);
    gsl_linalg_cholesky_decomp(sE.E2.L);

    // stats uncertainty 1*sigma principal axes
    EWS.decompSymm(U_dx, S_dx);
    for(size_t i=0; i<N; i++) gsl_vector_set(v1, i, sqrt(gsl_vector_get(S_dx,i)));

    rmul_diag(U_dx, v1);
    if(verbose > 1) { printf("U_dx:\n"); displayM(U_dx); }

    // update search region
    sE.calcCovering();              // ellipse containing both
    Quadratic Qs(N);                // A-form ellipse representation
    sE.EC.fillA(Qs);                // Cholesky to A
    QP.decompose(Qs);               // search region principal axes
    gsl_matrix_memcpy(dS, QP.USi);  // store to search region dS

    if(verbose) display();
}

void NoisyMin::fitMinSingular() {
    auto Q = fitHessian();      // update fit
    Q.fillA(U_q);
    EWS.decompSymm(U_q, S_q);   // Hessian principal axes SVD A = U_q D U_q^T, S_q = diag of D
    displayM(U_q);
    displayV(S_q);

    // repeat for uncertainty variations to determine stability along SVD axes:
    // calculate M2 = D' = U_q^T A' U_q ~ D for each variation; see how D'_jj varies.
    vector<double> dS_q(N); // maximum uncertainty magnitude for each component
    auto vQ = LMvariants();
    for(size_t i=0; i<NTERMS; i++) {
        vQ[i].fillA(M2);
        gsl_blas_dsymm(CblasLeft, CblasLower, 1, M2, U_q, 0, M1);
        gsl_blas_dgemm(CblasTrans, CblasNoTrans, 1, U_q, M1, 0, M2);
        for(size_t j = 0; j<N; j++) dS_q[j] = std::max(dS_q[j], fabs(gsl_matrix_get(M2,j,j) - gsl_vector_get(S_q,j)));
    }

    // determine good and singular subspaces
    vector<size_t> vG;  // good nonsingular axes in U_q
    vector<size_t> vS;  // singular subspace axes in U_q
    vector<double> vcontrib(N); // variable contributions to nonsingular space
    for(size_t j = 0; j<N; j++) {
        if(verbose) printf("\t%g ~ %g", gsl_vector_get(S_q,j), dS_q[j]);
        if(gsl_vector_get(S_q,j) - 2*dS_q[j] <= 0) vS.push_back(j);
        else {
            vG.push_back(j);
            for(size_t i=0; i<N; i++) vcontrib[i] += pow(gsl_matrix_get(U_q, i, j),2);
        }
    }
    auto nGood = vG.size();
    auto nBad = vS.size();
    if(verbose) {
        printf("\nFound %zu-dimensional nonsingular subspace.\nContributions:", nGood);
        for(auto c: vcontrib) printf("\t%g", c);
        printf("\n");
    }

    // consider 'good' subspace spanned by Qt = non-singular columns of Q
    // position in subspace x': x = Qt x', bt = Qt^T b
    // ellipse x'^T Qt^T A Qt x' + b^T Qt x' + c = 0 = x'^T Dt x' + bt^T x' + c
    auto Qt = gsl_matrix_alloc(N, nGood);
    for(size_t i=0; i<N; i++)
        for(size_t j=0; j<nGood; j++)
            gsl_matrix_set(Qt, i, j, gsl_matrix_get(U_q, i, vG[j]));

    // bt = Qt^T b
    auto bt = gsl_vector_alloc(nGood);
    vector2gsl(Q.b, v1);
    gsl_blas_dgemv(CblasTrans, 1, Qt, v1, 0, bt);

    // solve x0': Dt x0' = -bt/2
    vector<double> x0p(nGood);
    for(size_t j=0; j<nGood; j++) x0p[j] = -0.5 * gsl_vector_get(bt, j) / gsl_vector_get(S_q, vG[j]);
    if(verbose) {
        printf("x0' =");
        for(auto c: x0p) printf("\t%g", c);
        printf("\n");
    }

    // calculate x0' uncertainty ellipse for Qt^T A' Qt variations
    QuadraticCholesky Pg(nGood);
    auto Mg = gsl_matrix_calloc(nGood, nGood);
    auto ApQt = gsl_matrix_alloc(N,nGood);
    auto dx = gsl_vector_alloc(nGood);
    vec_t b(nGood);
    for(size_t i=0; i<NTERMS; i++) {
        vQ[i].fillA(M2);
        gsl_blas_dsymm(CblasLeft, CblasLower, 1, M2, Qt, 0, ApQt);
        gsl_blas_dgemm(CblasTrans, CblasNoTrans, 1, Qt, ApQt, 0, Pg.L);
        gsl_linalg_cholesky_decomp(Pg.L);
        vector2gsl(vQ[i].b, v1);
        gsl_blas_dgemv(CblasTrans, 1, Qt, v1, 0, bt);
        for(size_t j=0; j<nGood; j++) b[j] = gsl_vector_get(bt, j);
        Pg.findCenter(b, vQ[i].c);
        for(size_t j=0; j<nGood; j++) gsl_vector_set(dx, j, Pg.x0[j] - x0p[j]);
        gsl_blas_dsyr(CblasLower, 1., dx, Mg);
    }
    // x0' uncertainty principal vectors, magnitudes
    EigSymmWorkspace ESWg(nGood);
    ESWg.decompSymm(Mg, bt);
    if(verbose) { printf("dx0':\n"); displayM(Mg); displayV(bt); }

    // project initial fit range constraints into singular subspace
    ellipse_affine_projector EAP(N, nBad);
    for(size_t i=0; i<N; i++)
        for(size_t j=0; j<nBad; j++)
            gsl_matrix_set(EAP.TT, j, i, gsl_matrix_get(U_q, i, vS[j]));
    EAP.projectL(SR0.L, false); // EAP.P = U/S in Qbad space
    if(verbose) { printf("Ax:\n"); displayM(EAP.P); }
    // Mb = A' in Ub = U S^-2 U^T
    auto Mb = gsl_matrix_alloc(nBad,nBad);
    gsl_blas_dsyrk(CblasLower, CblasTrans, 1.0, EAP.P, 0., Mb);
    //fillSymmetric(CblasUpper, Mb); // not necessary: reconstructed A symmetric lower

    // reconstruct QC.L = A with limits on singular space
    gsl_matrix_set_zero(M1);
    for(size_t j=0; j<nGood; j++) gsl_matrix_set(M1, vG[j], vG[j], gsl_vector_get(S_q, vG[j]));
    for(size_t i=0; i<nBad; i++)
        for(size_t j=0; j<nBad; j++)
            gsl_matrix_set(M1, vS[i], vS[j], gsl_matrix_get(Mb, i, j));
    gsl_blas_dsymm(CblasLeft, CblasLower, 1, M1, U_q, 0, QC.L);
    gsl_blas_dgemm(CblasTrans, CblasNoTrans, 1, U_q, QC.L, 0, M1);
    displayM(M1);


    // cleanup
    gsl_matrix_free(Mg);
    gsl_matrix_free(Mb);
    gsl_matrix_free(Qt);
    gsl_matrix_free(ApQt);
    gsl_vector_free(bt);
}

void NoisyMin::display() {
    printf("NoisyMin fitter of %zu parameters wth %zu datapoints\n", N, fvals.size());
    for(size_t i=0; i<N; i++) {
        vec_t v(N);
        v[i] = 1.0;
        printf("[%zu]\t%g\t~%g (dh)\t~%g (stat)\n", i, x0[i], QC.projLength(v)*sqrt(h), sE.E2.projLength(v)/nSigmaStat);
    }
}
