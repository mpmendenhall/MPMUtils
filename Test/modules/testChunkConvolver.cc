/// @file testChunkConvolver.cc Convolution helper tests

#include "ChunkConvolver.hh"
#include "ConfigFactory.hh"

#include <stdlib.h>
#include <stdio.h>

/// print vector<double>
template<class V>
void display(const V& v) {
    for(auto x: v) {
        if(fabs(x) < 1e-10) x = 0;
        printf(" %4g", x);
    }
    printf("\n");
}

void cktest(size_t nkern) {
    vector<double> k0;
    for(size_t i=1; i <= nkern; ++i) k0.push_back(i);

    ChunkConvolver CC;
    CC.setKernel(k0);
    vector<double> vout;

    printf("\n\nKernel: "); display(k0);

    printf("\nVectors:\n");
    vector<vector<double>> vs;
    for(size_t i = 1; i < 5*nkern; ++i) {
        vector<double> v0(i);
        v0[0] = 1;
        if(i>1) v0.back() = 2.;
        display(v0);
        vs.push_back(v0);
    }

    printf("\nZero-padded convolutions:\n");
    for(auto& v0: vs) {
        CC.convolve(v0, vout);
        display(vout);
    }

    printf("\nFlat convolutions:\n");
    CC.boundaries[0] = CC.boundaries[1] = ChunkConvolver::BOUNDARY_FLAT;
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

REGISTER_EXECLET(testChunkConvolver) {
        for(size_t i=1; i <= 5; ++i) cktest(i);
}
