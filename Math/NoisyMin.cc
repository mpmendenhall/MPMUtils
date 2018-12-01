/// \file NoisyMin.cc

#include "NoisyMin.hh"
#include <gsl/gsl_cblas.h>
#include <gsl/gsl_eigen.h>
#include "GeomCalcUtils.hh"
#include <sstream>

NoisyMin::NoisyMin(size_t n): N(n),
NTERMS(Quadratic::nterms(N)), x0(N) {
    gsl_matrix_set_identity(dS);

    vnames.resize(N);
    int i=0;
    std::stringstream ss;
    for(auto& s: vnames) {
        ss << i++;
        s = ss.str();
    }
}

void NoisyMin::initRange() {
    x00 = x0;

    gsl_matrix_memcpy(SRm, dS);
    gsl_matrix_scale(SRm, 0.01);
    gsl_matrix_memcpy(sE.EC.L, dS);
    invert_colnorms(sE.EC.L);
    gsl_blas_dsyrk(CblasLower, CblasTrans, 1.0, sE.EC.L, 0., SR0);
    gsl_linalg_cholesky_decomp(SR0);
    gsl_matrix_memcpy(sE.EC.L, SR0);

    if(verbose) {
        printf("Initializing search range:\n");
        for(auto c: x0) printf("\t%g", c);
        printf("\n");
        displayM(dS);
    }
}

void NoisyMin::initMinStep() {
    minStep = true;
    if(verbose) { printf("Initializing minimum range:\n"); displayM(SRm); }
    gsl_matrix_memcpy(M1, SRm);
    invert_colnorms(M1);
    gsl_blas_dsyrk(CblasLower, CblasTrans, 1.0, M1, 0., SRm);
    gsl_linalg_cholesky_decomp(SRm);
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
    for(size_t i=0; i<N; i++) x[i] = v2(i);
    return x;
}

Quadratic NoisyMin::fitHessian() {
    // filter points to search region
    vector<evalpt> vs;
    for(auto& p: fvals) {
        for(size_t i=0; i<N; i++) v1(i) = p.x[i] - x0[i];
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
    if(verbose) {
        printf("Hessian fit:\n");
        Q.display();
    }
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
    for(size_t i=0; i<N; i++) for(size_t j=0; j<=i; j++) sE.E1.L(i, j) = QC.L(i, j)/sqrt(h);

    // fit parameter uncertainties to minimum location uncertainty
    auto vQ = LMvariants();
    for(size_t i=0; i<NTERMS; i++) {
        QC.decompose(vQ[i]);
        for(size_t j=0; j<N; j++) v1(j) = QC.x0[j] - x0[j];
        gsl_blas_dsyr(CblasLower, 1., v1, U_dx);
    }

    // n-sigma stats uncertainty inverse Cholesky form
    for(size_t i=0; i<N; i++) for(size_t j=0; j<=i; j++) sE.E2.L(i, j) = U_dx(i, j)*nSigmaStat*nSigmaStat;
    gsl_linalg_cholesky_decomp(sE.E2.L);
    gsl_linalg_cholesky_invert(sE.E2.L);
    gsl_linalg_cholesky_decomp(sE.E2.L);

    // stats uncertainty 1*sigma principal axes
    EWS.decompSymm(U_dx, S_dx);
    for(size_t i=0; i<N; i++) v1(i) = sqrt(S_dx(i));

    rmul_diag(U_dx, v1);
    if(verbose > 1) { printf("U_dx:\n"); displayM(U_dx); }

    updateRange();

    if(verbose) display();
}

void NoisyMin::fitMinSingular() {
    auto Q = fitHessian();      // update fit
    Q.fillA(U_q);
    EWS.decompSymm(U_q, S_q);   // Hessian principal axes SVD A = U_q D U_q^T, S_q = diag of D
    if(verbose) {
        printf("\nHessian principal axes (columns):\n");
        displayM(U_q);
    }

    // repeat for uncertainty variations to determine stability along SVD axes:
    // calculate M2 = D' = U_q^T A' U_q ~ D for each variation; see how D'_jj varies.
    auto vQ = LMvariants(); // variant Hessians within fit uncertainties
    vector<double> dS_q(N); // maximum uncertainty magnitude for each component
    for(size_t i=0; i<NTERMS; i++) {
        vQ[i].fillA(M2);
        gsl_blas_dsymm(CblasLeft, CblasLower, 1, M2, U_q, 0, M1);
        gsl_blas_dgemm(CblasTrans, CblasNoTrans, 1, U_q, M1, 0, M2);
        for(size_t j = 0; j<N; j++) dS_q[j] = std::max(dS_q[j], fabs(M2(j,j) - S_q(j)));
    }

    // determine good and singular subspaces
    vector<size_t> vG;  // good nonsingular axes in U_q
    vector<size_t> vS;  // singular subspace axes in U_q
    vector<double> vcontrib(N); // variable contributions to nonsingular space
    if(verbose) printf("Hessian principal axes 1/width^2:\n");
    for(size_t j = 0; j<N; j++) {
        if(verbose) printf("\t%g ~ %g\n", S_q(j), dS_q[j]);
        if(S_q(j) - 2*dS_q[j] <= 0) vS.push_back(j);
        else {
            vG.push_back(j);
            for(size_t i=0; i<N; i++) vcontrib[i] += pow(U_q(i, j),2);
        }
    }
    auto nGood = vG.size();
    auto nBad = vS.size();
    if(verbose) {
        printf("\nParameter contributions to %zu-dimensional nonsingular subspace:\n", nGood);
        size_t i = 0;
        for(auto c: vcontrib) printf("\t%24s :\t%g\n", vnames[i++].c_str(), c);
        printf("\n");
    }

    // special case: entirely singular; do not update values
    if(!nGood) return;

    // determine best fit point x0' in 'good' suspace
    // position in subspace x': x = Qt x', bt = Qt^T b
    // ellipse x'^T Qt^T A Qt x' + b^T Qt x' + c = 0 = x'^T Dt x' + bt^T x' + c
    gsl_matrix_wrapper Qt(N, nGood);
    for(size_t i=0; i<N; i++)
        for(size_t j=0; j<nGood; j++)
            Qt(i, j) = U_q(i, vG[j]);

    // bt = Qt^T b
    gsl_vector_wrapper bt(nGood);
    vector2gsl(Q.b, v1);
    gsl_blas_dgemv(CblasTrans, 1, Qt, v1, 0, bt);

    // solve x0': Dt x0' = -bt/2
    vector<double> x0p(nGood);
    for(size_t j=0; j<nGood; j++) x0p[j] = -0.5 * bt(j) / S_q(vG[j]);
    if(verbose) {
        printf("good subspace x0' =");
        for(auto c: x0p) printf("\t%g", c);
        printf("\n");
    }

    // TODO: coerce iffy x0' values into fit range

    // project initial fit range constraints into singular subspace
    ellipse_affine_projector EAP(N, nBad);
    for(size_t i=0; i<N; i++)
        for(size_t j=0; j<nBad; j++)
            EAP.TT(j, i) = U_q(i, vS[j]);
    EAP.projectL(SR0, false); // EAP.P = U/S in Qbad space
    // Mb = A' in Ub = U S^-2 U^T
    gsl_matrix_wrapper Mb(nBad,nBad);
    if(nBad) gsl_blas_dsyrk(CblasLower, CblasTrans, 1.0, EAP.P, 0., Mb);
    //fillSymmetric(CblasUpper, Mb); // not necessary: reconstructed A symmetric lower

    // project previous best fit into 'bad' subspace, x0b = Qbad^T * x0
    gsl_matrix_wrapper Qb(N, nBad);
    for(size_t i=0; i<N; i++)
        for(size_t j=0; j<nBad; j++)
            Qb(i, j) = U_q(i, vS[j]);
    gsl_vector_wrapper x0b(nBad);
    vector2gsl(x0, v1);
    if(nBad) gsl_blas_dgemv(CblasTrans, 1, Qb, v1, 0, x0b);
    if(verbose) { printf("bad subspace x0' = "); displayV(x0b); }

    // untransform x0 = U_q x' best-fit point
    for(size_t i=0; i<nGood; i++) v1(vG[i]) = x0p[i];
    for(size_t i=0; i<nBad; i++) v1(vS[i]) = x0b(i);
    if(nBad) gsl_blas_dgemv(CblasNoTrans, 1, U_q, v1, 0, v2);
    gsl2vector(v2, x0);
    k0 = Q(x0);

    // calculate x0' uncertainty ellipse (and k0 uncertainty) for Qt^T A' Qt variations
    QuadraticCholesky Pg(nGood);
    gsl_matrix_wrapper Mg(nGood, nGood); // uncertainty ellipse A' = sum dx' dx'^T
    gsl_matrix_wrapper Mg2(nGood, nGood);
    gsl_matrix_wrapper ApQt(N,nGood);
    gsl_vector_wrapper dx(nGood);
    vec_t b(nGood);
    dk2 = 0;
    for(size_t i=0; i<NTERMS; i++) {
        dk2 += pow(vQ[i](x0) - k0, 2);
        vQ[i].fillA(M2);
        gsl_blas_dsymm(CblasLeft, CblasLower, 1, M2, Qt, 0, ApQt);
        gsl_blas_dgemm(CblasTrans, CblasNoTrans, 1, Qt, ApQt, 0, Pg.L);
        gsl_linalg_cholesky_decomp(Pg.L);
        vector2gsl(vQ[i].b, v1);
        gsl_blas_dgemv(CblasTrans, 1, Qt, v1, 0, bt);
        for(size_t j=0; j<nGood; j++) b[j] = bt(j);
        Pg.findCenter(b, vQ[i].c);
        for(size_t j=0; j<nGood; j++) {
            if(!(Pg.x0[j] == Pg.x0[j])) Pg.x0[j] = x0p[j]; // NaN check!
            dx(j) = Pg.x0[j] - x0p[j];
        }
        gsl_blas_dsyr(CblasLower, 1., dx, Mg2);
    }
    // x0' uncertainty principal vectors * l^2
    EigSymmWorkspace ESWg(nGood);
    if(verbose) displayM(Mg2);
    ESWg.decompSymm(Mg2, bt);
    // convert to Hessian form: A = (U/l) (U/l)^T
    for(size_t i=0; i<nGood; i++) bt(i) = 1/sqrt(bt(i));
    rmul_diag(Mg2, bt);
    gsl_blas_dsyrk(CblasLower, CblasNoTrans, 1., Mg2, 0., Mg);
    if(verbose) { printf("good subspace dx0' Hessian:\n"); displayM(Mg); displayV(bt); }


    // convert stat. uncertainty to nSigmaStat ellipse in full space with projected limits on singular subspace
    // fill in nSigmaStat-scaled 'good' subspace
    gsl_matrix_set_zero(sE.E1.L);
    for(size_t i=0; i<nGood; i++)
        for(size_t j=0; j<=i; j++)
            sE.E1.L(vG[i], vG[j]) = Mg(i, j)/(nSigmaStat*nSigmaStat);
    // fill in projected limits on 'bad' subspace
    for(size_t i=0; i<nBad; i++)
        for(size_t j=0; j<=i; j++)
            sE.E1.L(vS[i], vS[j]) = Mb(i, j);
    // sE.E1.L = A = U_q A' U_q^T
    gsl_blas_dsymm(CblasRight, CblasLower, 1, sE.E1.L, U_q, 0, M1);
    gsl_blas_dgemm(CblasNoTrans, CblasTrans, 1, M1, U_q, 0, sE.E1.L);
    displayM(sE.E1.L);
    // Cholesky decompose
    gsl_linalg_cholesky_decomp(sE.E1.L);

    // convert dh limit on good subspace to ellipse in full space with projected limits on singular subspace
    gsl_matrix_set_zero(sE.E2.L);
    for(size_t j=0; j<nGood; j++) sE.E2.L(vG[j], vG[j]) = S_q(vG[j])/h;
    // fill in projected limits on 'bad' subspace
    for(size_t i=0; i<nBad; i++)
        for(size_t j=0; j<=i; j++)
            sE.E2.L(vS[i], vS[j]) = Mb(i, j);
    // sE.E2.L = A = U_q A' U_q^T
    gsl_blas_dsymm(CblasRight, CblasLower, 1, sE.E2.L, U_q, 0, M1);
    gsl_blas_dgemm(CblasNoTrans, CblasTrans, 1, M1, U_q, 0, sE.E2.L);
    displayM(sE.E2.L);
    // Cholesky decompose
    gsl_linalg_cholesky_decomp(sE.E2.L);

    if(verbose) printf("\nMinimum value: %g +- %g\n", k0, sqrt(dk2));

    updateRange();
}

void NoisyMin::updateRange() {
    sE.calcCovering();              // cover both stat and h ranges
    if(minStep) {                   // expand to minimum range
        gsl_matrix_memcpy(sE.E1.L, sE.EC.L);
        gsl_matrix_memcpy(sE.E2.L, SRm);
        sE.calcCovering(true);
    }
    // clip to initial (maximum) range
    gsl_matrix_memcpy(sE.E1.L, sE.EC.L);
    gsl_matrix_memcpy(sE.E2.L, SR0);
    sE.calcCovering(false);



    Quadratic Qs(N);                // A-form ellipse representation
    sE.EC.fillA(Qs);                // Cholesky to A
    QP.decompose(Qs);               // search region principal axes
    gsl_matrix_memcpy(dS, QP.USi);  // store to search region dS

    if(verbose) {
        printf("\n");
        display();
        double v = 1;
        for(size_t i=0; i<N; i++) v *= QP.Si(i);
        printf("\nUpdating search range (volume %g):\n", v);
        displayM(dS);
    }
}

void NoisyMin::display() {
    printf("NoisyMin fitter of %zu parameters wth %zu datapoints\n", N, fvals.size());
    for(size_t i=0; i<N; i++) {
        vec_t v(N);
        v[i] = 1.0;
        printf("%24s :\t%g\t~%g (dh)\t~%g (stat)\n", vnames[i].c_str(), x0[i], sE.E2.projLength(v), sE.E1.projLength(v)/nSigmaStat);
    }
}

std::ostream& operator<<(std::ostream& o, const NoisyMin::evalpt& p) {
    for(auto c: p.x) o << c << "\t";
    o << p.f << "\t" << p.df2 << "\n";
    return o;
}
