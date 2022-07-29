/// \file testFFTW.cc FFTW3 wrapper tests

#include "FFTW_Convolver.hh"
#include "ConfigFactory.hh"
#include "TermColor.hh"

#include <stdlib.h>
#include <stdio.h>

// calculation precision type
//typedef float calcs_t;       // 32 bit
typedef double calcs_t;      // 64 bit; min_exponent = -1021
//typedef long double calcs_t; // 80 bits stored in 128; min_exponent = -16381
//typedef __float128 calcs_t;  // 128 bit

// std::complex type
typedef fftwx<calcs_t>::scplx_t cplx_t;

/// print real or complex
void printx(calcs_t x) { printf("%5g", x); }
/// print real or complex
void printx(cplx_t x) {  printf("(%5g %+5gi)", x.real(), x.imag()); }

/// print real or complex vector
template<class V>
void display(const V& v) {
    for(auto x: v) { printf(" "); printx(x); }
    printf("\n");
}

/// test round-trip
template<class Plan>
void test_roundtrip(const vector<calcs_t>& v) {
    IFFTWorkspace<Plan> W(v.size());
    W.v_x.assign(v.begin(), v.end());
    W.execute();
    W.etucexe();
    printf(" -> "); display(W.v_x);
}

template<class Plan>
FFTWorkspace<Plan>& show_xform(const vector<calcs_t>& v) {
    display(v);
    auto& P = FFTWorkspace<Plan>::get_ffter(v.size(), true);
    P.v_x.assign(v.begin(), v.end());
    P.execute();
    printf("is"); display(P.v_k);
    return P;
}

/// R-to-C using C-to-C
void show_R_C2C(const vector<calcs_t>& v) {
    printf("DFT of "); show_xform<DFTPlan<calcs_t>>(v);
}

/// Show real-to-complex DFT
FFTWorkspace<R2CPlan<calcs_t>>& show_R2C(const vector<calcs_t>& v) {
    printf("R2C of "); return show_xform<R2CPlan<calcs_t>>(v);
}

/// Real-to-real symmetric FFT
template<class RR>
void show_R2R(const vector<calcs_t>& v) {
    printf("R2R of "); show_xform<RR>(v);
}

template<class V>
void test_roundtrips(const V& v) {
    printf("Round - trips on "); display(v);
    test_roundtrip<R2CPlan<calcs_t>>(v);
    test_roundtrip<DCT_I_Plan<calcs_t>>(v);
    test_roundtrip<DCT_II_Plan<calcs_t>>(v);
    test_roundtrip<DCT_III_Plan<calcs_t>>(v);
    test_roundtrip<DCT_IV_Plan<calcs_t>>(v);
    test_roundtrip<DST_I_Plan<calcs_t>>(v);
    test_roundtrip<DST_II_Plan<calcs_t>>(v);
    test_roundtrip<DST_III_Plan<calcs_t>>(v);
    test_roundtrip<DST_IV_Plan<calcs_t>>(v);
}

void yprint(const char* s) {
    printf("\n" TERMFG_YELLOW "%s" TERMSGR_RESET "\n\n", s);
}

/// Apply RC-style time constant tau (sample spacings) to input vector
template<class V>
void ZFilter(V& v, double tau, double poles = 1) {
    // forward calculation, symmetrizing v:
    auto& P = IFFTWorkspace<DCT_I_Plan<calcs_t>>::get_ffter(v.size(), true);
    P.v_x = v;
    P.execute();

    // convert to complex-valued k-space
    const auto N = v.size();
    auto& Fi = IFFTWorkspace<R2CPlan<calcs_t>>::get_iffter(2*N - 2);
    Fi.v_k = P.v_k;

    // apply filter response
    int k = 0;
    for(auto& c: Fi.v_k) {
        auto w = k*2*M_PI/N;
        cplx_t u(1. - w*w*tau, 0);
        c *= std::pow(u, poles);
        k++;
    }
    // return to real space; truncate to initial non-symmetrized region
    Fi.etucexe();

    v.assign(Fi.v_x.begin(), Fi.v_x.end());

    //k = 0;
    //for(auto& x: v) x = Fi.v_x[k++];
}


#include <TGraph.h>
#include <TPad.h>

REGISTER_EXECLET(testFFTW) {
    printf("\nsizeof(calcs_t) = %zu, min_exponent = %i\n\n", sizeof(calcs_t),
           std::numeric_limits<calcs_t>::min_exponent);

    vector<calcs_t> v3({1., 2.5, 3.});
    //vector<calcs_t> v3k({0.3, 1., 2.});

    vector<calcs_t> v4({1., 2., 3.7, 4.1});
    vector<calcs_t> v5({1., 2., 5., 3., 4.1});
    vector<calcs_t> v6({1., 2., 5., 3., 4., 6.7});
    //vector<calcs_t> v6k({0., 1., 2., 0., 3., 0.});

    test_roundtrips(v3);

    yprint("--- Real DFT as Hermitian-symmetric case of DFT ---");

    show_R_C2C(v4);
    show_R2C(v4);

    printf("\n--------------------\n\n");

    show_R_C2C(v5);
    show_R2C(v5);

    yprint("--- DCT-I (a bcd e dcb) : k-space real, symmetric ---");

    show_R2C(symmetrize_o(v5));
    printf("DCT I of "); show_xform<DCT_I_Plan<calcs_t>>(v5);

    printf("\n--------------------\n\n");

    show_R2C(symmetrize_o(v6));
    printf("DCT I of "); show_xform<DCT_I_Plan<calcs_t>>(v6);

    yprint("--- DCT-II (0 a 0 b 0 c 0 c 0 b 0 a) : half-sample interleaving ---");

    show_R2C(interzero(symmetrize_e(v3)));
    printf("DCT II of "); show_xform<DCT_II_Plan<calcs_t>>(v3);

    yprint("--- DCT-III (abc 0 CB A BC 0 cb): factor-of-0.5 from R2C ---");

    show_R2C(symmetrize_o(asymmetrize_o(v3)));
    printf("DCT III of "); show_xform<DCT_III_Plan<calcs_t>>(v3);

    yprint("--- DCT-IV (0 a 0 b 0 c 0 C 0 B 0 A 0 A 0 B 0 C 0 c 0 b 0 a): factor-of-0.5 from R2C ---");

    show_R2C(interzero(symmetrize_e(asymmetrize_e(v3))));
    printf("DCT IV of "); show_xform<DCT_IV_Plan<calcs_t>>(v3);

    yprint("--- DST-I (0 abc 0 CBA) : k-space imaginary, antisymmetric ---");

    show_R2C(asymmetrize_o(v5, true));
    printf("DST I of "); show_xform<DST_I_Plan<calcs_t>>(v5);

    yprint("--- DST-II (0 a 0 b 0 c 0 C 0 B 0 A) ---");

    show_R2C(interzero(asymmetrize_e(v3)));
    printf("DST II of "); show_xform<DST_II_Plan<calcs_t>>(v3);

    yprint("--- DST-III (0 abcba 0 ABCBA) : k-space 0-interleaved, factor of -0.5 ---");

    show_R2C(dstIIIsymm(v3));
    printf("DST III of "); show_xform<DST_III_Plan<calcs_t>>(v3);

    yprint("--- DST-IV ---");

    show_R2C(dstIVsymm(v3));
    printf("DST IV of "); show_xform<DST_IV_Plan<calcs_t>>(v3);


    yprint("--- Convolutions ---");

    GaussConvolverFactory<calcs_t> GCF(0.5);
    GaussDerivFactory<calcs_t> GDF(0.5);

    vector<calcs_t> delta1(10);
    for(int j=0; j<10; ++j) delta1[j] = 0.1*j;
    GDF.convolve(delta1);
    display(delta1);

    for(int i=0; i<10; ++i) {
        vector<calcs_t> delta2(10);
        delta2[i] += 1;
        GDF.convolve(delta2);
        display(delta2);
    }

    yprint("--- RC filter ---");

    size_t N = 128;
    vector<calcs_t> d(N); d[N/4] = 1;
    auto dd = d;
    ZFilter(dd, 16, 1);

    auto dx = dd;
    N = dd.size();
    while(N--) dx[N] = N;

    //display(d);
    //display(dd);

    //TGraph g1(d.size(), dx.data(), d.data());
    TGraph g2(dd.size(), dx.data(), dd.data());

    g2.SetLineColor(2);
    g2.Draw("AL");
    //g1.Draw("L");
    gPad->Print("Filtered.pdf");
}
