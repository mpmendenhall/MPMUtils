/// \file testLaplacianSum.cc Test of LaplacianSums functions

#include "ConfigFactory.hh"
#include "LaplacianSums.hh"
#include <stdio.h>
#include <TStopwatch.h>

void test_lapsum(double a, double b, double c) {
    TStopwatch sw;

    sw.Start();

    double s;
    int ntest = 1e4;
    for(int i=0; i<ntest; ++i) {
        s = 1./c;
        for(int k = 1; k < 10000; ++k) s += 1./(a*k*k + b*k + c) + 1./(a*k*k - b*k + c);
    }
    printf("%g in %g ns", s, 1e9*sw.CpuTime()/ntest);

    sw.Start();
    ntest = 1e7;
    for(int i=0; i<ntest; ++i) s = sum_inverse_quadratic(a,b,c);
    printf(" vs %g in %g ns\n", s, 1e9*sw.CpuTime()/ntest);
}

REGISTER_EXECLET(testLapsum) {
    test_lapsum(3,.3,2);
    test_lapsum(3,12,2);
}
