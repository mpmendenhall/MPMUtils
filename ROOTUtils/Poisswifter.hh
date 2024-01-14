/// @file Poisswifter.hh Faster binned Poisson distribution generator
// -- Michael P. Mendenhall, LLNL 2021

#ifndef POISSWIFTER_HH
#define POISSWIFTER_HH

#include <TRandom.h>
#include <vector>
using std::vector;

/// Faster binned Poisson distribution generator
class Poisswifter {
public:
    /// Constructor
    explicit Poisswifter(TRandom& _R): R(_R) { }

    /// convert Poisson expectation values to counts
    void toCounts(vector<double>& v);

    /// test generator speed for optimizing calculation strategy
    void speedTest();

protected:
    double xover = 40.;  ///< strategy crossover point

    TRandom& R;
    vector<double> cprob;
    vector<int> ibins;
};

#endif
