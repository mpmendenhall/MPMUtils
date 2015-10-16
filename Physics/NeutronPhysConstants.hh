/// \file NeutronPhysConstants.hh Particle physics constants relevant to neutron decay

#ifndef NEUTRONPHYSCONSTANTS_HH
#define NEUTRONPHYSCONSTANTS_HH

// PDG values: K.A. Olive et al. (Particle Data Group), Chin. Phys. C, 38, 090001 (2014) and 2015 update

const double m_e = 510.998928;                  ///< +-(11), electron mass [keV/c^2]. PDG 2014 = CODATA 2010
const double delta_mn_mp = 1293.33217;          ///< +-(42), m_n-m_p mass difference [keV/c^2]. PDG 2014 = CODATA 2010
const double neutronBetaEp = delta_mn_mp - m_e; ///< neutron beta decay endpoint [keV]
const double m_p = 938272.046;                  ///< +-(21), proton mass [keV/c^2]. PDG 2014 = CODATA 2010
const double m_n = m_p + delta_mn_mp;           ///< neutron mass [keV/c^2]
const double alpha = .0072973525664;            ///< +-(17), fine structure constant. CODATA 2014
const double lambda = +1.2723;                  ///< +-(23), lambda = |g_A/g_V|, PDG 2014 value, Wilkinson sign convention
const double A0_PDG = -0.1184;                  ///< +-(10), beta decay asymmetry A_0, PDG 2014
const double beta_W0 = delta_mn_mp/m_e;         ///< neutron beta decay energy, ``natural'' units
const double neutron_R0 = 0.0025896*1.2;        ///< neutron and proton radius approximation, in "natural" units (1.2fm)/(hbar/m_e*c)
const double proton_M0 = m_p/m_e;               ///< proton mass, ``natural'' units
const double neutron_M0 = m_n/m_e;              ///< neutron mass, ``natural'' units
const double gamma_euler = 0.577215;            ///< Euler's constant

/// a_0 calculated from given lambda
inline double calc_a0(double l = lambda) { return (1-l*l)/(1+3*l*l); }

#endif
