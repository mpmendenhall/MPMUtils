/// \file NuclPhysConstants.hh Nuclear physics constants

#ifndef NUCLPHYSCONSTANTS_HH
#define NUCLPHYSCONSTANTS_HH

// PDG values: K.A. Olive et al. (Particle Data Group), Chinese Physics C, 38, 090001 (2014) and 2015 update

namespace physconst {
    const double m_e = 0.5109989461;                ///< +-(31), electron mass [MeV/c^2] from CODATA 2014

    const double delta_mn_mp = 1.29333217;          ///< +-(42), m_n-m_p mass difference [MeV/c^2]. PDG 2014 = CODATA 2010
    const double neutronBetaEp = delta_mn_mp - m_e; ///< neutron beta decay endpoint [MeV]
    const double m_p = 938.272046;                  ///< +-(21), proton mass [MeV/c^2]. PDG 2014 = CODATA 2010
    const double m_n = m_p + delta_mn_mp;           ///< neutron mass [MeV/c^2]
    const double m_amu = 931.4940954;               ///< +-(57) atomic mass unit (AMU) in MeV/c^2 CODATA 2014
    
    const double m_deuteron = 1875.612928;          ///< +-(12), deuteron mass [MeV/c^2] from CODATA 2014
    const double m_triton = 2808.921112;            ///< +-(17), triton mass [MeV/c^2] from CODATA 2014
    const double m_alpha = 3727.379378;             ///< +-(23), alpha particle mass [MeV/c^2] from CODATA 2014
    
    const double alpha = .0072973525664;            ///< +-(17), fine structure constant. CODATA 2014
    const double gamma_euler = 0.57721566490153286060651209008240243104215933593992;    ///< Euler's constant (to 50 digits)

    const double c_mps = 299792458.;                ///< speed of light in vacuum [m/s], exact.
}

#endif
