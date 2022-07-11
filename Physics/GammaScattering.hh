/// \file GammaScattering.hh Gamma-electron scattering calculations
// Michael P. Mendenhall, LLNL 2022

#ifndef GAMMASCATTERING_HH
#define GAMMASCATTERING_HH

#include <cmath>
#include <limits>
#include "NuclPhysConstants.hh"

using namespace physconst;
/// electron radius cross section [barn]
constexpr double cx_e = 0.01 * M_PI * r_e*r_e;

/////////////////
// a := Ein/m_e
// x := cos(theta)  = 1 +  m_e / E_in - m_e / E_out
// ds/dx = 2 pi ds/dO
// f := Eout/Ein = 1/(1 + a*(1-x))   =>   x = 1 - (1/f - 1)/a
// df/dx = a/(1+a*(1-x))^2
// ds/df = ds/dx / (df/dx) = pi r_e^2 f^2 (1+a*(1-x))^2 (f + 1/f + x^2 - 1) / a

/// Photon kinematics f = E_out/E_in for scattering into angle cos(theta)
inline double gamma_escatter_f(double Ein_per_m_e, double cth) { return 1/(1 + (1-cth)*Ein_per_m_e); }
/// Photon kinematics scattering angle x = cos(theta) for outgoing energy fraction f
inline double gamma_escatter_cth(double Ein_per_m_e, double f) { return 1 - (1/f-1)/Ein_per_m_e; }
/// Compton scattering minimum f = E_out / E_in (= maximum 1-f transferred to electron)
inline double gamma_escatter_fmin(double Ein_per_m_e) { return 1/(1 + 2*Ein_per_m_e); }
/// Compton scattering maximimum E_in / m_e that can scatter down to E_out
inline double gamma_escatter_Emax_per_m_e(double Eout_per_m_e) {
    return Eout_per_m_e < 0.5? Eout_per_m_e/(1 - 2*Eout_per_m_e) : std::numeric_limits<double>::infinity();
}

/// Unpolarized Klein–Nishina gamma-electron angular scattering cross-section ds/dx [barn / cos theta]
/// given precalculated f = E_out / E_in, x = cos(theta)
inline double _KN_ds_dcth(double f, double x) {
    return cx_e * f * (1 + f*(f + x*x - 1));
}

/// Unpolarized Klein–Nishina gamma-electron angular scattering cross-section ds/dcos(theta)
/// given incident gamma energy E_in / m_e and scattering angle x = cos(theta)
inline double KN_ds_dcth(double Ein_per_m_e, double x) {
    return _KN_ds_dcth(gamma_escatter_f(Ein_per_m_e, x), x);
}

/// Unpolarized Klein–Nishina scattering cross-section ds/df [barn / f]
/// given precalculated f, x = cth
inline double _KN_ds_df(double a, double f, double x) {
    double u = (1 + a*(1-x));
    return cx_e * f * (1 + f*(f + x*x - 1)) * u*u / a;
}

/// Unpolarized Klein–Nishina scattering cross-section ds/df [barn / f]
/// given incident gamma energy E_in / m_e and outgoing gamma energy fraction f = E_out/E_in
inline double KN_ds_df(double Ein_per_m_e, double f) {
    return _KN_ds_df(Ein_per_m_e, f, gamma_escatter_cth(Ein_per_m_e, f));
}

/// Total gamma/electron cross section into all scattering angles, int ds/dx dx [barn]
//  sigma_tot = pi * r_e^2 int (f^3 - f^2 + f + f^2 x^2) dx
//  f = 1/(1 + a*(1-x))
//  let u := 1-x  =>  int_{-1}^1 dx = int_0^2 du; f = 1/(1 + a*u)
inline double KN_total_xs(double a /* = Ein_per_m_e */) {
    if(!a) return 8*cx_e/3;
    double b = 1 + 2*a;
    return cx_e * (2*a*(2 + a*(a+1)*(a+8))/(b*b) + (a*(a-2)-2)*log(b))/(a*a*a);
}


/// Polarized Klein–Nishina gamma-electron angular scattering, given cos theta and cos^2 phi
inline double KN_ds_dcth(double Ein_MeV, double cth, double c2phi) {
    double fE = gamma_escatter_f(Ein_MeV/m_e, cth);
    return cx_e * fE*fE * (fE + 1/fE - 2 * (1 - cth*cth) * c2phi);
}

#endif
