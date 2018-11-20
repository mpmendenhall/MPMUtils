/// \file testQuadratic.cc Test quadratic manipulations

#include "CodeVersion.hh"
#include "Quadratic.hh"
#include "Visr.hh"
#include <vector>
using std::vector;
#include <TGraph.h>
#include <TPad.h>
#include <TRandom3.h>

void* visthread(void*) {
    vsr::doGlutLoop();
    return NULL;
}

/// Plot out ellipse slice in specified axis plane from columns basis m
TGraph vEllipse(const gsl_matrix* m, double x0, double y0, size_t ax = 0, size_t ay = 1, int npts = 100) {
    TGraph g(npts+1);

    auto v = gsl_vector_calloc(m->size1);
    auto v2 = gsl_vector_calloc(m->size1);

    for(int k=0; k <= npts; k++) {
        auto th = k*2*M_PI/npts;
        auto c = cos(th);
        auto s = sin(th);
        gsl_vector_set(v, ax, c);
        gsl_vector_set(v, ay, s);
        gsl_blas_dgemv(CblasNoTrans, 1., m, v, 0., v2);
        g.SetPoint(k, x0 + gsl_vector_get(v2, ax), y0 + gsl_vector_get(v2, ay));
    }

    gsl_vector_free(v);
    gsl_vector_free(v2);
    return g;
}

/// show 3-ellipse in vsr from principle axis columns
void visEllipse(const gsl_matrix* m, size_t ax0 = 0, size_t ax1 = 1, size_t ax2 = 2) {

    auto v = gsl_vector_calloc(m->size1);
    auto v2 = gsl_vector_calloc(m->size1);

    size_t axperms[3][3] = {{ax0,ax1,ax2}, {ax2,ax0,ax1}, {ax1,ax2,ax0}};

    int npts = 50;
    vector<double> vc(npts), vs(npts);
    for(int k=0; k < npts; k++) {
            auto th = k*2*M_PI/npts;
            vc[k] = cos(th);
            vs[k] = sin(th);
    }

    for(auto na: {0,1,2}) {
        auto a = axperms[na][0];
        auto a1 = axperms[na][1];
        auto a2 = axperms[na][2];

        vsr::vec3 x = {gsl_matrix_get(m, ax0, a), gsl_matrix_get(m, ax1, a), gsl_matrix_get(m, ax2, a)};
        auto mx = x;
        for(auto& c: mx) c = -c;
        //vsr::setColor(1,0,0);
        vsr::line(mx, x);

        //vsr::setColor(0,0,1,0.3);
        vsr::startLines();
        int nsp = 11;
        for(int k=0; k <= nsp*npts; k++) {
            auto z = (k-npts*nsp/2.)*2/(nsp*npts);
            gsl_vector_set(v, a, z);
            gsl_vector_set(v, a1, sqrt(1.-z*z)*vc[k % npts]);
            gsl_vector_set(v, a2, sqrt(1.-z*z)*vs[k % npts]);
            gsl_blas_dgemv(CblasNoTrans, 1., m, v, 0., v2);

            x = {gsl_vector_get(v2, 0), gsl_vector_get(v2, 1), gsl_vector_get(v2, 2)};
            vsr::vertex(x);
        }
        vsr::endLines();
    }
}

void visProj(QuadraticCholesky<3>& QC, int ax0 = 0, int ax1 = 1, int ax2 = 2) {

    auto v = gsl_vector_calloc(2);
    auto v2 = gsl_vector_calloc(2);

    int axperms[3][3] = {{ax0,ax1,ax2}, {ax2,ax0,ax1}, {ax1,ax2,ax0}};
    int npts = 100;
    vector<double> vc(npts), vs(npts);
    for(int k=0; k <= npts; k++) {
            auto th = k*2*M_PI/npts;
            vc[k] = cos(th);
            vs[k] = sin(th);
    }

    for(auto na: {0,1,2}) {
        auto a = axperms[na][0];
        auto a1 = axperms[na][1];
        auto a2 = axperms[na][2];

        ellipse_affine_projector EAP(3,2);
        EAP.setAxes(vector<int>({a1,a2}));
        EAP.projectL(QC.L);
        displayM(EAP.P);

        vsr::setColor(1,0,0);
        vsr::vec3 x = {};
        x[a] = 1;
        x[a] *= QC.projLength(x);
        vsr::vec3 x2 = x;
        x2[a] *= -1;
        vsr::line(x2, x);

        vsr::setColor(0,1,0);
        vsr::startLines();
        for(int k=0; k <= npts; k++) {
            gsl_vector_set(v, 0, vc[k]);
            gsl_vector_set(v, 1, vs[k]);
            gsl_blas_dgemv(CblasNoTrans, 1., EAP.P, v, 0., v2);

            x[a] = 0;
            x[a1] = gsl_vector_get(v2, 0);
            x[a2] = gsl_vector_get(v2, 1);
            vsr::vertex(x);
        }
        vsr::endLines();
    }

}

int main(int, char**) {
    CodeVersion::display_code_version();

    vsr::initWindow("Ellipses!", 0.2);
    pthread_t thread;
    pthread_create(&thread, NULL, &visthread, nullptr );


    Quadratic<3> R(vector<double>({1.,2.,3.,4.,5.,6.,7.,8.,9.,10.}));
    R *= 0.5;
    R += R;
    R.display();

    QuadraticCholesky<3> QC;
    QC.decompose(R);
    QC.display();

    vector<double> x0 = {-5.7, -0.6, 1.4};
    printf("%g\n", R(x0));

    array<double,10> c;
    Quadratic<3>::evalTerms(x0, c);
    for(auto x: c) printf("\t%g",x);
    printf("\n");

    TRandom3 TR;

    while(true) {

        try{
            vector<double> vr = {1., 0 , 1, 0, 0, 1, 0, 0, 0, 0};
            //vector<double> vr(Quadratic<3>::NTERMS);
            for(auto& c: vr) c = 0.5 + TR.Uniform();
            R = Quadratic<3>(vr);
            for(auto& c: vr) c = 0.5 + TR.Uniform();
            //vr = {1, 1 , 1, 0, 0, 0.5, 0, 0, 0, 0};
            Quadratic<3> R2(vr);
            R.display();
            R2.display();



            QuadraticPCA<3> QP, QP2, QPc;
            QP.decompose(R);
            QP2.decompose(R2);


            //int a1 = 0, a2 = 1;
            //auto g = vEllipse(QP.USi, QC.x0[a1], QC.x0[a2], a1, a2);
            //g2.Draw("AL");
            //g.Draw("AL");
            //gPad->Print("ellipse.pdf");


            QC.decompose(R);
            //ellipse_affine_projector EAP(3,2);
            //EAP.setAxes(vector<int>({a1,a2}));
            //EAP.projectL(QC.L);
            //auto g2 = vEllipse(EAP.P, QC.x0[a1], QC.x0[a2]);
            //g2.SetLineColor(2);

            CoveringEllipse<3> CE;
            CE.E1.calcCholesky(R);
            CE.E2.calcCholesky(R2);
            CE.calcCovering(true);
            Quadratic<3> Rc;
            CE.EC.fillA(Rc);
            QPc.decompose(Rc);

            displayM(QP.USi);
            displayM(QP2.USi);

            vsr::startRecording();
            vsr::clearWindow();

            vsr::setColor(0,0,1,0.3);
            visEllipse(QP.USi);

            //vsr::setColor(0,1,0,0.3);
            //visEllipse(QP2.USi);

            //vsr::setColor(1,0,0,0.3);
            //visEllipse(QPc.USi);

            visProj(QC);

            vsr::stopRecording();

            vsr::pause();
        } catch(...) { printf("Ouch, try again!\n"); }

    }

    vsr::set_kill();
    pthread_join(thread, nullptr);

    return EXIT_SUCCESS;
}
