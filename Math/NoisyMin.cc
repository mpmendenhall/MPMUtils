/// \file NoisyMin.cc

#include "NoisyMin.hh"
#include <gsl/gsl_cblas.h>
#include <gsl/gsl_eigen.h>
#include <sstream>

NoisyMin::NoisyMin(size_t n): N(n),
NTERMS(Quadratic::nterms(N)), x0(N) {
    gsl_matrix_set_identity(dS);

    int i = 0;
    std::stringstream ss;
    for(auto& s: vnames) {
        ss << i++;
        s = ss.str();
    }
}

void NoisyMin::initRange() {
    gsl_matrix_memcpy(SRm, dS);
    gsl_matrix_scale(SRm, 0.01);
    gsl_matrix_memcpy(sE.EC.L, dS);
    invert_colnorms(sE.EC.L);
    gsl_blas_dsyrk(CblasLower, CblasTrans, 1.0, sE.EC.L, 0., SR0);
    gsl_linalg_cholesky_decomp(SR0);
    gsl_matrix_memcpy(sE.EC.L, SR0);

    if(verbose) displaySearchRange();
}

void NoisyMin::displaySearchRange() const {
    printf("Search range:\nx0 = ");
    for(auto c: x0) printf("\t%g", c);
    printf("\n");
    displayM(dS);
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
    assert(Ntot <= N);
    if(Ntot < N) addPart(N-Ntot, Quadratic::nterms(N-Ntot));

    vec_t p0 = next();
    for(auto& x: p0) x = 2*x-1;
    // TODO spherize subgroups

    // update by partition groups, not quite correct:
    // |AB|x -> |A|x,     x
    // |CD|y          |CD|y
    vec_t x(N);
    auto rit = p0.begin();
    auto xit = x0.begin();
    size_t j = 0;
    for(auto& p: parts) {
        gsl_vector_wrapper vp0(p.N+j);
        if(vp0) std::copy(rit, rit+p.N+j, vp0->data);

        gsl_vector_wrapper vx0(p.N);
        if(vx0) std::copy(xit, xit+p.N, vx0->data);
        xit += p.N;

        gsl_matrix_wrapper dSi(p.N, j + p.N);
        for(size_t r = 0; r < p.N; r++)
            for(size_t c = 0; c < j+p.N; c++)
                dSi(r,c) = dS(j+r, c);

        // vx0 = vx0 + nsigma * dS * vp0
        gsl_blas_dgemv(CblasNoTrans, nsigma, dSi, vp0, 1., vx0);
        for(size_t i=0; i<p.N; i++) x.at(j++) = vx0(i);
    }

    return x;
}

Quadratic NoisyMin::fitHessian() {
    if(verbose) displaySearchRange();

    // filter points to search region
    vector<evalpt> vs;
    for(auto& p: fvals) {
        for(size_t i=0; i<N; i++) v1(i) = p.x[i] - x0[i];
        gsl_blas_dtrmv(CblasLower, CblasTrans, CblasNonUnit, sE.EC.L, v1);
        if(gsl_blas_dnrm2(v1) < 1.001) vs.push_back(p);
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
    auto s2 = LM.nDF() > 0? LM.ssresid()/LM.nDF() : 0;
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
    k0 = Q(x0);

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
    if(verbose) printf("Hessian principal axes widths:\n");
    for(size_t j = 0; j<N; j++) {
        if(verbose) printf("\t%g ~ %g\n", 1/sqrt(S_q(j)), 0.5*dS_q[j]*pow(S_q(j), -1.5));
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
    gsl_blas_dgemv(CblasNoTrans, 1, U_q, v1, 0, v2);
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
    PointSelector::display();
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

std::istream& operator>>(std::istream& i, NoisyMin::evalpt& p) {
    for(auto& c: p.x) i >> c;
    Quadratic::evalTerms(p.x, p.t);
    i >> p.f >> p.df2;
    return i;
}

std::ostream& operator<< (std::ostream &o, const NoisyMin& NM) {
    o << NM.N << '\n';
    o << (const PointSelector&)NM;
    o << NM.dS << NM.U_dx << NM.S_dx << NM.U_q << NM.S_q << NM.SR0 << NM.SRm;
    o << NM.h << '\t' << NM.verbose << '\t' << NM.nSigmaStat << '\t' << NM.k0 << '\t' << NM.dk2 << '\t' << NM.minStep << '\n';

    o << NM.fvals.size() << '\n';
    for(auto& p: NM.fvals) o << p;

    o << NM.QRNGn << '\n';

    // internal / debugging quantities
    //vector<std::string> vnames;     ///< variable names
    //LinMin LM{NTERMS};              ///< fitter for quadratic surface x^T A x + b^T x + c around minimum

    return o;
}

std::istream& operator>> (std::istream &i, NoisyMin& NM) {
    size_t n = 0;
    i >> n;
    NM = NoisyMin(n);
    i >> (PointSelector&)NM;
    i >> NM.dS >> NM.U_dx >> NM.S_dx >> NM.U_q >> NM.S_q >> NM.SR0 >> NM.SRm;
    i >> NM.h >> NM.verbose >> NM.nSigmaStat >> NM.k0 >> NM.dk2 >> NM.minStep;

    i >> n;
    while(n--) {
        NM.fvals.emplace_back(NM.N);
        i >> NM.fvals.back();
    }

    i >> NM.QRNGn;
    NM.QRNG.Skip(NM.QRNGn);

    return i;
}
