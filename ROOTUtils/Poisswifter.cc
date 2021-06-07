/// \file Poisswifter.cc

#include "Poisswifter.hh"
#include <TStopwatch.h>
#include <stdexcept>

void Poisswifter::toCounts(vector<double>& v) {
    // allocate temporary vectors as needed
    ibins.resize(v.size());
    cprob.resize(v.size()+1);
    cprob[0] = 0;

    // scan input vector
    int nlo = 0;
    int i = 0;
    for(auto& l: v) {
        if(l > xover) l = R.Poisson(l);
        else if(l > 0) {
            ibins[nlo++] = i;
            cprob[nlo] = cprob[nlo-1] + l;
            l = 0;
        }
        ++i;
    }

    if(!nlo) return;

    cprob.resize(nlo+1);
    auto ptot = cprob.back();
    int llo = R.Poisson(ptot);
    while(llo--) {
        auto n = std::lower_bound(cprob.begin(), cprob.end(), R.Uniform(ptot)) - cprob.begin();
        if(n == nlo) --n;
        ++v[ibins[n]];
    }

    // clear temporary vectors
    ibins.clear();
    cprob.clear();
}

void Poisswifter::speedTest() {
    TStopwatch sw;

    int ntest = 100000000;
    double to_ns = 1e9/ntest;

    sw.Start();
    for(int i = 0; i < ntest; ++i) R.Uniform();
    double t_unif = sw.CpuTime()*to_ns;
    printf("Uniform: %g ns per call\n", t_unif);

    ntest /= 10;
    to_ns = 1e9/ntest;
    for(auto l: {1e-3, 1e-2, .1, 1., 10., 20., 30., 40., 50., 60., 70., 80., 90., 100., 1000., 10000., 100000.}) {
        sw.Start();
        for(int i = 0; i < ntest; ++i) R.Poisson(l);
        double t_poiss = sw.CpuTime()*to_ns;
        printf("Poisson(%g): %g\tusing uniform: %g\n", l, t_poiss, l*t_unif);
    }
}

