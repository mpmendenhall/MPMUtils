/// \file testFFTW.cc FFTW3 wrapper tests

#include "FFTW_Convolver.hh"
#include "ConfigFactory.hh"

#include <stdlib.h>
#include <stdio.h>

// calculation precision type
//typedef float calcs_t;       // 32 bit
typedef double calcs_t;      // 64 bit; min_exponent = -1021
//typedef long double calcs_t; // 80 bits stored in 128; min_exponent = -16381
//typedef __float128 calcs_t;  // 128 bit

// std::complex type
typedef fftwx<calcs_t>::scplx_t cplx_t;

/// print real vector
template<class V>
void display(const V& v) {
    for(auto x: v) printf("\t%5g", (double)x);
    printf("\n");
}

/// print complex vector
template<class V>
void cdisplay(const V& v) {
    for(auto& x: v) printf("  (%5g %+5gi)", (double)x.real(), (double)x.imag());
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

/// R-to-C using C-to-C
void show_R_C2C(const vector<calcs_t>& v) {
    printf("DFT of "); display(v);
    auto& P = DFTWorkspace<calcs_t>::get_ffter(v.size(), true);
    P.v_x.assign(v.begin(), v.end());
    P.execute();
    printf("is"); cdisplay(P.v_k);
}

/// Show real-to-complex DFT
void show_R2C(const vector<calcs_t>& v) {
    printf("R2C of "); display(v);
    auto& P = R2CWorkspace<calcs_t>::get_ffter(v.size(), true);
    P.v_x.assign(v.begin(), v.end());
    P.execute();
    printf("is"); cdisplay(P.v_k);
}

/// Real-to-real symmetric FFT
template<class RR>
void show_R2R(const vector<calcs_t>& v) {
    printf("R2R of "); display(v);
    FFTWorkspace<RR> P(v.size(), true);
    P.v_x.assign(v.begin(), v.end());
    P.execute();
    printf("is"); display(P.v_k);
}

/// symmetrize around center element abcd -> abc d cb
template<class V>
V symmetrize_o(const V& v) {
    V v2(v);
    for(int i = int(v.size()-2); i > 0; --i) v2.push_back(v[i]);
    return v2;
}

/// mirror-symmetrize abc -> abc cba
template<class V>
V symmetrize_e(const V& v) {
    V v2(v);
    for(int i = int(v.size()-1); i >= 0; --i) v2.push_back(v[i]);
    return v2;
}

/// zero-interleave (half-samples)
template<class V>
V interzero(const V& v) {
    V v2;
    for(auto x: v) { v2.push_back(0); v2.push_back(x); }
    return v2;
}

/// antisymmetrize abc -> abc BCA
template<class V>
V asymmetrize_e(const V& v) {
    V v2(v);
    for(int i = int(v.size()-1); i >= 0; --i) v2.push_back(-v[i]);
    return v2;
}

/// antisymmetrize abc -> abc 0 CBA [0]
template<class V>
V asymmetrize_o(const V& v, bool fzero = false) {
    V v2;
    if(fzero) v2.push_back(0);
    for(auto x: v) v2.push_back(x);
    v2.push_back(0);
    for(int i = int(v.size()-1); i >= 0; --i) v2.push_back(-v[i]);
    return v2;
}

/// duplicate negated abc -> abc ABC
template<class V>
V dupneg(const V& v) {
    V v2(v);
    for(auto x: v) v2.push_back(-x);
    return v2;
}

/// DST-III symmetry abc -> 0 abcba 0 ABCBA
template<class V>
V dstIIIsymm(const V& v) {
    V v2;
    v2.push_back(0);
    for(auto x: v) v2.push_back(x);
    for(int i = int(v.size()-2); i >= 0; --i) v2.push_back(v[i]);
    return dupneg(v2);
}

/// DST-IV symmetry
template<class V>
V dstIVsymm(const V& v) { return dupneg(interzero(symmetrize_e(v))); }

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

REGISTER_EXECLET(testFFTW) {
    printf("\nsizeof(calcs_t) = %zu, min_exponent = %i\n\n", sizeof(calcs_t),
           std::numeric_limits<calcs_t>::min_exponent);

    vector<calcs_t> v3({1., 2.5, 3.});
    vector<calcs_t> v3k({0.3, 1., 2.});

    vector<calcs_t> v4({1., 2., 3.7, 4.1});
    vector<calcs_t> v5({1., 2., 5., 3., 4.1});
    vector<calcs_t> v6({1., 2., 5., 3., 4., 6.7});
    vector<calcs_t> v6k({0., 1., 2., 0., 3., 0.});

    test_roundtrips(v3);

    printf("\n--- Real DFT as Hermitian-symmetric case of DFT ---\n\n");

    show_R_C2C(v4);
    show_R2C(v4);

    printf("\n--------------------\n\n");

    show_R_C2C(v5);
    show_R2C(v5);

    printf("\n\n--- DCT-I (a bcd e dcb) : k-space real, symmetric ---\n\n");

    show_R2C(symmetrize_o(v5));
    show_R2R<DCT_I_Plan<calcs_t>>(v5);

    printf("\n--------------------\n\n");

    show_R2C(symmetrize_o(v6));
    show_R2R<DCT_I_Plan<calcs_t>>(v6);

    printf("\n--- DCT-II (0 a 0 b 0 c 0 c 0 b 0 a) : half-sample interleaving ---\n\n");

    show_R2C(interzero(symmetrize_e(v3)));
    show_R2R<DCT_II_Plan<calcs_t>>(v3);

    printf("\n--- DCT-III (abc 0 CB A BC 0 cb): factor-of-0.5 from R2C ---\n\n");

    show_R2C(symmetrize_o(asymmetrize_o(v3)));
    show_R2R<DCT_III_Plan<calcs_t>>(v3);

    printf("\n--- DCT-IV (0 a 0 b 0 c 0 C 0 B 0 A 0 A 0 B 0 C 0 c 0 b 0 a): factor-of-0.5 from R2C ---\n\n");

    show_R2C(interzero(symmetrize_e(asymmetrize_e(v3))));
    show_R2R<DCT_IV_Plan<calcs_t>>(v3);

    printf("\n\n--- DST-I (0 abc 0 CBA) : k-space imaginary, antisymmetric ---\n\n");

    show_R2C(asymmetrize_o(v5, true));
    show_R2R<DST_I_Plan<calcs_t>>(v5);

    printf("\n\n--- DST-II (0 a 0 b 0 c 0 C 0 B 0 A) ---\n\n");

    show_R2C(interzero(asymmetrize_e(v3)));
    show_R2R<DST_II_Plan<calcs_t>>(v3);

    printf("\n\n--- DST-III (0 abcba 0 ABCBA) : k-space 0-interleaved, factor of -0.5 ---\n\n");

    show_R2C(dstIIIsymm(v3));
    show_R2R<DST_III_Plan<calcs_t>>(v3);

    printf("\n\n--- DST-IV ---\n\n");

    show_R2C(dstIVsymm(v3));
    show_R2R<DST_IV_Plan<calcs_t>>(v3);


    printf("\n\n--- Convolutions ---\n\n");

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
}
