/// \file testRational.cc Test rational number functions

#include "CodeVersion.hh"
#include "Eratosthenes.hh"
#include "Rational.hh"
#include "TestOperators.hh"
#include "Abstract.hh"
#include <stdio.h>
#include <iostream>
#include <cassert>
#include <random>

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

    Rational a;
    for(int i=1; i<=20; i++) {
        Rational b(i%2? 1 : -1, i);
        testAdd(a,b);
    }
    a += 2;

    PolynomialV_t<void,Rational> Pr(1,0);
    Pr += Rational(1,2);
    cout << Pr << Pr.pow(5) << "\n";

    auto& PS = theSieve();
    summary(PS);
    for(PrimeSieve::int_t i=0; i<=10000000; i++) {
        auto v = PS.factor(i);
        assert(i == PS.prod(v));
        if(rand() > 1e-5*RAND_MAX) continue;
        printf("%i =", PS.prod(v));
        for(auto f: v) printf("\t%i", f);
        printf("\n");
        summary(PS);
    }

    return EXIT_SUCCESS;
}
