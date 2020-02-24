/// \file FFTW_Convolver.cc

#include "FFTW_Convolver.hh"

template<>
std::mutex& fftw_planner_mutex<double>() { static std::mutex l; return l; }
template<>
std::mutex& fftw_planner_mutex<float>() { static std::mutex l; return l; }
template<>
std::mutex& fftw_planner_mutex<long double>() { static std::mutex l; return l; }
