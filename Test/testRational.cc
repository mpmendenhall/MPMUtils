/// \file testRational.cc Test rational number functions

#include "CodeVersion.hh"
#include "Eratosthenes.hh"
#include <stdio.h>

void summary(const PrimeSieve& S) {
    auto& ps = S.getPrimes();
    auto& xf = S.getXf();
    printf("%zu primes and %zu extra factorizations:", ps.size(), xf.size());
    for(auto p: ps) printf("\t%i", p);
    printf("\n");
    printf("Xs:");
    for(auto& kv: xf) printf("\t%i", kv.first);
    printf("\n");
}

int main(int, char**) {
    CodeVersion::display_code_version();

    auto& PS = theSieve();
    summary(PS);
    PrimeSieve::int_t i = 1234567893;
    auto v = PS.factor(i);
    printf("%i =", PS.prod(v));
    for(auto f: v) printf("\t%i", f);
    printf("\n");
    summary(PS);

    return EXIT_SUCCESS;
}
