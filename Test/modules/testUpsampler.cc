/// @file testUpsampler.cc Upsampler helper tests

// ./bin/mpmexamples testUpsampler

#include "Upsampler.hh"
#include "ConfigFactory.hh"

#include <stdlib.h>
#include <stdio.h>

/// print vector<double>
template<class V>
void display(const V& v) {
    for(auto x: v) printf(" %+0.2f", x);
    printf("\n");
}

REGISTER_EXECLET(testUpsampler) {

    vector<double> v0({11., 9., 12., 8.});

    Upsampler U;
    for(size_t n = 1; n <= 8; ++n) {
        U.set_sinc_interpolator(n);
        printf("\nKernel [%zu]: ", n);
        display(U.getKernel());
        vector<double> vout;
        U.upsample(v0, vout);
        display(vout);
    }
}
