/// \file UnpolarizedBeta.hh Unpolarized nucleus beta decay spectrum/corrections
// -- Michael P. Mendenhall, LLNL 2020

#ifndef UNPOLARIZEDBETA_HH
#define UNPOLARIZEDBETA_HH

#include <cmath>
#include "NuclPhysConstants.hh"
using namespace physconst;

const double proton_M0 = m_p/m_e;               ///< proton mass, ``natural'' units
const double neutron_M0 = m_n/m_e;              ///< neutron mass, ``natural'' units
const double beta_W0 = delta_mn_mp/m_e;         ///< neutron beta decay energy, ``natural'' units
const double neutron_R0 = 0.0025896*1.2;        ///< neutron and proton radius approximation, in "natural" units (1.2fm)/(hbar/m_e*c)

const double A0_PDG = -0.1184;                  ///< +-(10), beta decay asymmetry A_0, PDG 2014
const double lambda = 1.2723;                   ///< +-(23), lambda = |g_A/g_V|, PDG 2014 value, Wilkinson sign convention
const double delta_mu = 2.792847356-(-1.91304273);  ///< mu_p - mu_n = 2.792847356(23) - -1.91304273(45) PDG 2010

/// a_0 calculated from given lambda
inline double calc_a0(double l = lambda) { return (1-l*l)/(1+3*l*l); }

/// dilog function, L = -int_0^x ln(1-z)/z dz = gsl_sf_dilog(x)
double dilog(double x);
/// Spence function with positive sign convention, L = +int_0^x ln(1-z)/z dz
inline double SpenceL(double x) { return -dilog(x); }

//-------------- Spectrum corrections ------------------

// references:
// [0] Wilkinson, Analysis of Neutron Beta-Decay, Nucl. Phys. A 377 (1982) 474-504
// [1] Wilkinson, Evaluation of Beta-Decay I,   NIM A 275 (1989) 378-386
// [2] Wilkinson, Evaluation of Beta-Decay II,  NIM A 290 (1990) 509-515
// [3] Wilkinson, Evaluation of Beta-Decay III, NIM A 335 (1995) 305-309
// [4] Wilkinson, Evaluation of Beta-Decay IV,  NIM A 365 (1995) 203-207
// [5] Wilkinson, Evaluation of Beta-Decay V,   NIM A 365 (1995) 497-507

// NOTE: functions of W are using Wilkinson's ``natural'' units for energy, W=(KE+m_e)/m_e

/// beta decay phase space without corrections
inline double plainPhaseSpace(double W, double W0=beta_W0) { return (1.<W && W<W0)?sqrt(W*W-1)*W*(W0-W)*(W0-W):0; }
/// integral from 1 to W of plainPhaseSpace
double plainPhaseSpaceCDF(double W, double W0=beta_W0);
/// beta for particle with given KE
inline double beta(double KE, double m = m_e) { return sqrt(1-m*m/((KE+m)*(KE+m))); }

/// lowest order approximation of F
inline double crudeF(double Z, double W) { return 1+M_PI*alpha*Z*W/sqrt(W*W-1.); }
/// power series approximation of F(Z,W;R) in [1]
double WilkinsonF_PowerSeries(double Z, double W, double R=neutron_R0);
/// Wilkinson's F0(Z,W;R) as in [0],[1],[2],[3]; using complex gamma approximation to N terms
double WilkinsonF0(double Z, double W, double R = neutron_R0, unsigned int N = 3);

/// L_0(Z,W) as parametrized in [2], correction to point-like point charge used for F0(Z,W)
double WilkinsonL0(double Z, double W, double R = neutron_R0);

/// R(W,W0,M) as parametrized in [2], phase space correction for nuclear recoil, Vector part
double WilkinsonRV(double W, double W0=beta_W0, double M=proton_M0);
/// R(W,W0,M) as parametrized in [2], phase space correction for nuclear recoil, Axial Vector part
double WilkinsonRA(double W, double W0=beta_W0, double M=proton_M0);
/// Combined Vector/Axial-Vector nuclear recoil correction to spectrum shape
double CombinedR(double W, double M2_F, double M2_GT, double W0=beta_W0, double M=proton_M0);

/// correction to spectrum shape from recoil + weak magnetism according to Bilenkii 1959, eq. 11, after factoring out (1+3*lambda^2)
double Bilenkii59_RWM(double W);

/// Wilkinson ^VC(Z,W) as in [2] nucleon/lepton wavefunction convolution correction, Vector part
double WilkinsonVC(double Z, double W, double W0=beta_W0, double R=neutron_R0);
/// Wilkinson ^AC(Z,W) as in [2], nucleon/lepton wavefunction convolution correction, axial part
double WilkinsonAC(double Z, double W, double W0=beta_W0, double R=neutron_R0);
/// Combined Vector/Axial-Vector C
double CombinedC(double Z, double W, double M2_F, double M2_GT, double W0=beta_W0, double R=neutron_R0);

/// Wilkinson Q(Z,W,M) as in [0], nucleon recoil effect on Coulomb corrections
double WilkinsonQ(double Z,double W,double W0=beta_W0,double M=proton_M0);

/// Sirlin 1967 g * alpha/2pi radiative corrections to order alpha, also in [5]
double Sirlin_g_a2pi(double KE,double KE0,double m=m_e);
/// Wilkinson g * alpha/2pi: Sirlin g + fix for logarithm divergence [5]
double Wilkinson_g_a2pi(double W,double W0=beta_W0, double M=proton_M0);

/// shape factor for first forbidden Tensor/Axial decays, per J. Davidson, Phys. Rev. 82(1) p. 48, 1951
double Davidson_C1T(double W, double W0, double Z, double R);
/// shape factor for Cs137 second forbidden beta decay, per L.M. Langer and R.J.D. Moffat,  Phys. Rev. 82(5), p. 635, 1951
double Langer_Cs137_C2T(double W, double W0);
/// shape factor for Cs137 second forbidden beta decay, per H. Behrens and P. Christmas, Nucl. Phys. A399, pp. 131-140, 1983
double Behrens_Cs137_C(double W, double W0);

/// combined spectrum correction factor for unpolarized neutron beta decay
double neutronSpectrumCorrectionFactor(double KE);
/// corrected beta spectrum for unpolarized neutron beta decay
double neutronCorrectedBetaSpectrum(double KE);

#endif
