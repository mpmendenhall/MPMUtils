/// \file FFTW_Convolver.cc

#include "FFTW_Convolver.hh"

template<>
std::mutex& ConvolvePlan<double>::plannerLock() { static std::mutex l; return l; }
template<>
std::mutex& ConvolvePlan<float>::plannerLock() { static std::mutex l; return l; }
template<>
std::mutex& ConvolvePlan<long double>::plannerLock() { static std::mutex l; return l; }
