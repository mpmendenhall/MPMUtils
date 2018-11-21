/// \file NoisyMin.cc

#include "NoisyMin.hh"

NoisyMin::~NoisyMin() {
    for(auto m: {Udx0, dS, U_q}) gsl_matrix_free(m);
    for(auto m: Mt) gsl_matrix_free(m);
    for(auto v: {Sdx0, S_q, dS_q, v1, v2}) gsl_vector_free(v);
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

void NoisyMin::fitHessian() {
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
    Q.setCoeffs(y);  // estimated minimum surface
    if(verbose) Q.display();
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
    fitHessian();           // update Q
    QChol.decompose(Q);     // find minimum position (and Cholesky form)
    if(verbose) QChol.display();
    x0 = QChol.x0;
    for(size_t i=0; i<N; i++) for(size_t j=0; j<=i; j++) gsl_matrix_set(sE.E1.L, i, j, gsl_matrix_get(QChol.L, i, j)/sqrt(h));

    // fit parameter uncertainties to minimum location uncertainty
    auto vQ = LMvariants();
    for(size_t i=0; i<NTERMS; i++) {
        QC.decompose(vQ[i]);
        for(size_t j=0; j<N; j++) gsl_vector_set(v1, j, QC.x0[j] - x0[j]);
        gsl_blas_dsyr(CblasLower, 1., v1, Udx0);
    }

    // n-sigma stats uncertainty inverse Cholesky form
    for(size_t i=0; i<N; i++) for(size_t j=0; j<=i; j++) gsl_matrix_set(sE.E2.L, i, j, gsl_matrix_get(Udx0, i, j)*nSigmaStat*nSigmaStat);
    gsl_linalg_cholesky_decomp(sE.E2.L);
    gsl_linalg_cholesky_invert(sE.E2.L);
    gsl_linalg_cholesky_decomp(sE.E2.L);

    // stats uncertainty 1*sigma principal axes
    EWS.decompSymm(Udx0, Sdx0);
    for(size_t i=0; i<N; i++) gsl_vector_set(v1, i, sqrt(gsl_vector_get(Sdx0,i)));

    rmul_diag(Udx0, v1);
    if(verbose > 1) { printf("Udx0:\n"); displayM(Udx0); }

    // update search region
    sE.calcCovering();              // ellipse containing both
    Quadratic Qs(N);                // A-form ellipse representation
    sE.EC.fillA(Qs);                // Cholesky to A
    QP.decompose(Qs);               // search region principal axes
    gsl_matrix_memcpy(dS, QP.USi);  // store to search region dS

    if(verbose) display();
}

void NoisyMin::fitMinSingular() {
    fitHessian();               // update Q
    Q.fillA(U_q);
    EWS.decompSymm(U_q, S_q);   // Hessian principal axes SVD A = Q D Q^T
    displayM(U_q);
    displayV(S_q);

    // repeat for uncertainty variations to determine stability along SVD axes
    vector<double> dS_q(N); // maximum uncertainty magnitude for each component
    auto vQ = LMvariants();
    for(size_t i=0; i<NTERMS; i++) {
        // calculate Mt[i] = Q^T A' Q for this variation
        vQ[i].fillA(Mt[i]);
        gsl_blas_dsymm(CblasLeft, CblasLower, 1, Mt[i], U_q, 0, M1);
        gsl_blas_dgemm(CblasTrans, CblasNoTrans, 1, U_q, M1, 0, Mt[i]);
        for(size_t j = 0; j<N; j++) dS_q[j] = std::max(dS_q[j], fabs(gsl_matrix_get(Mt[i],j,j) - gsl_vector_get(S_q,j)));
    }

    // determine good and singular subspaces
    vector<size_t> vG;  // good nonsingular axes
    vector<size_t> vS;  // singular subspace axes
    for(size_t j = 0; j<N; j++) {
        printf("\t%g ~ %g", gsl_vector_get(S_q,j), dS_q[j]);
        if(gsl_vector_get(S_q,j) - 2*dS_q[j] <= 0) vS.push_back(j);
        else vG.push_back(j);
    }
    printf("\n");
    if(verbose) printf("Found %zu-dimensional nonsingular subspace.\n", vG.size());

    // calculate x0 uncertainty ellipse in good subspace
    auto Mg = gsl_matrix_alloc(vG.size(), vG.size());
    for(size_t i=0; i<NTERMS; i++) {

    }
    gsl_matrix_free(Mg);

    // trim bad subspace to initial fit range

    printf("\n");
}

void NoisyMin::display() {
    printf("NoisyMin fitter of %zu parameters wth %zu datapoints\n", N, fvals.size());
    for(size_t i=0; i<N; i++) {
        vec_t v(N);
        v[i] = 1.0;
        printf("[%zu]\t%g\t~%g (dh)\t~%g (stat)\n", i, x0[i], QChol.projLength(v)*sqrt(h), sE.E2.projLength(v)/nSigmaStat);
    }
}
