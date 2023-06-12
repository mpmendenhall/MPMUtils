/// \file testChunkConvolver.cc Convolution helper tests

#include "ChunkConvolver.hh"
#include "ConfigFactory.hh"

#include <stdlib.h>
#include <stdio.h>

/// print vector<double>
template<class V>
void display(const V& v) {
    for(auto x: v) printf(" %4.0f", x);
    printf("\n");
}

REGISTER_EXECLET(testChunkConvolver) {

    vector<double> k0({1., 2., 3.});

    ChunkConvolver CC;
    CC.setKernel(k0);
    vector<double> vout;

    printf("Kernel: "); display(k0);

    printf("\nVectors:\n");
    vector<vector<double>> vs;
    for(int i=1; i<10; ++i) {
        vector<double> v0(i);
        v0[0] = 1;
        v0.back() = 1.;
        display(v0);
        vs.push_back(v0);
    }

    printf("\nZero-padded convolutions:\n");
    for(auto& v0: vs) {
        CC.convolve(v0, vout);
        display(vout);
    }

    printf("\nCyclic convolutions:\n");
    CC.boundaries[0] = CC.boundaries[1] = ChunkConvolver::BOUNDARY_WRAP;
    for(auto& v0: vs) {
        CC.convolve(v0, vout);
        display(vout);
    }
}
