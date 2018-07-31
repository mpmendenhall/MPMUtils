/// \file testRational.cc Test rational number functions

#include "CodeVersion.hh"
#include "Eratosthenes.hh"
#include <stdio.h>
#include <cassert>

void summary(const PrimeSieve& S) {
    auto& ps = S.getPrimes();
    auto& xf = S.getXf();
    printf("%zu primes (out of %i) and %zu extra factorizations:", ps.size(), S.maxchecked(), xf.size());
    //for(auto p: ps) printf("\t%i", p);
    printf("\n");

    if(xf.size() > 10) return;
    printf("Xs:");
    for(auto& kv: xf) printf("\t%i", kv.first);
    printf("\n");
}

int main(int, char**) {
    CodeVersion::display_code_version();

    auto& PS = theSieve();
    summary(PS);
    for(PrimeSieve::int_t i=0; i<=10000000; i++) {
        auto v = PS.factor(i);
        assert(i == PS.prod(v));
        if(i%1000001) continue;
        printf("%i =", PS.prod(v));
        for(auto f: v) printf("\t%i", f);
        printf("\n");
        summary(PS);
    }

    return EXIT_SUCCESS;
}
