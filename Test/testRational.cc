/// \file testRational.cc Test rational numbers and associated fields

#include "CodeVersion.hh"
#include "Eratosthenes.hh"
#include "Rational.hh"
#include "TestOperators.hh"
#include "SurdField.hh"
#include "ModularField.hh"
#include "Abstract.hh"
#include "Quaternion.hh"
#include "Stopwatch.hh"
#include "ProgressBar.hh"

#include <stdio.h>
#include <iostream>
#include <cassert>
#include <random>

void summary(const PrimeSieve& S) {
    auto& ps = S.getPrimes();
    auto& xf = S.getXf();
    printf("%zu primes (out of %lli) and %zu extra factorizations:", ps.size(), S.maxchecked(), xf.size());
    //for(auto p: ps) printf("\t%i", p);
    printf("\n");

    if(xf.size() > 10) return;
    printf("Xs:");
    for(auto& kv: xf) printf("\t%lli", kv.first);
    printf("\n");
}

int main(int, char**) {
    CodeVersion::display_code_version();

    auto& PS = theSieve();
    summary(PS);
    {
        Stopwatch w;
        ProgressBar PB(1000000);
        PrimeSieve::int_t i=0;
        while(!++PB) {
            auto v = PS.factor(i);
            if(i != PrimeSieve::int_t(PS.prod(v))) throw std::runtime_error("Factoring fail!");
            ++i;

            if(rand() > 1e-5*RAND_MAX) continue;
            printf("%lli =", PS.prod(v));
            for(auto f: v) printf("\t%lli", f);
            printf("\n");
            summary(PS);
        }
    }
    summary(PS);

    auto& primes = PS.getPrimes();
    auto& pdivs  = PS.getPDivs();
    size_t nd0 = 0, nd1 = 0;
    {
        Stopwatch w;
        ProgressBar PB(1000000);
        PrimeSieve::int_t i=0;
        while(!++PB) {
            for(auto p: primes) nd0 += !(i%p);
            ++i;
        }
        printf("%zu divided (%% check)\n", nd0);
    }
    {
        Stopwatch w;
        ProgressBar PB(1000000);
        PrimeSieve::int_t i=0;
        while(!++PB) {
            for(auto& np: pdivs) nd1 += np.divides(i);
            ++i;
        }
        printf("%zu divided (fast check)\n", nd1);
    }
    if(nd0 != nd1) throw std::runtime_error("Fast divisor check fail!");

    Rational a;
    for(int i=1; i<=20; i++) {
        Rational b(i%2? 1 : -1, i);
        testAdd(a,b);
    }
    a += 2;

    PolynomialV_t<Rational> Pr(1,0);
    Pr += {1,2};
    cout << Pr << Pr.pow(5) << "\n";

    auto r3 = SurdSum::sqrt(3);
    r3 /= 8;
    auto r5 = (SurdSum::sqrt(-5) + 1)/2;
    auto r35 = SurdSum(1)/(SurdSum::sqrt(2) + SurdSum::sqrt(3) + SurdSum::sqrt(5) + SurdSum::sqrt(84));
    r35.invert();
    cout << r5 << " . " << r5.inverse() + 1 << " . " << (r5 + r3)*(r5 - r3) << " & " << r35 << "\n";
    //r3 /= 0;

    auto ii = EuclidRelPrime(1027,712);
    std::cout << ii.first << " " << ii.second << "\n";

    typedef Quaternion<Rational> Quat;
    Quat Q({1},{2},{3},{4});
    std::cout << Q << "\n" << Q*Q << "\n" << Q.inverse() << "\n" << Q/Q << "\n";

    return EXIT_SUCCESS;
}
