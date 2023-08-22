/// \file NuclPhysConstants.hh Nuclear physics constants
// -- Michael P. Mendenhall

#ifndef NUCLPHYSCONSTANTS_HH
#define NUCLPHYSCONSTANTS_HH

// CODATA 2018: https://physics.nist.gov/cuu/Constants/index.html

/// Useful nuclear physics constants
namespace physconst {
    constexpr double m_e = 0.51099895000;               ///< +-(15), electron mass           [MeV/c^2] CODATA 2018
    constexpr double m_mu  = 105.6583755;               ///< +-(23), muon mass               [MeV/c^2] CODATA 2018
    constexpr double m_amu = 931.49410242;              ///< +-(28), atomic mass unit (AMU)  [MeV/c^2] CODATA 2018
    constexpr double m_p = 938.27208816;                ///< +-(29), proton mass             [MeV/c^2] CODATA 2018
    constexpr double m_deuteron = 1875.61294257;        ///< +-(57), deuteron (2H) mass      [MeV/c^2] CODATA 2018
    constexpr double m_helion = 2808.39160743;          ///< +-(85), helion (3He) mass       [MeV/c^2] CODATA 2018
    constexpr double m_triton = 2808.92113298;          ///< +-(85), triton (3H) mass        [MeV/c^2] CODATA 2018
    constexpr double m_alpha = 3727.3794066;            ///< +-(11), alpha (4He) mass        [MeV/c^2] CODATA 2018
    constexpr double m_6Li   = 5603.0509;               ///< +-(5),  6Li mass                [MeV/c^2] warning, needs cited reference!

    constexpr double delta_mn_mp = 1.29333236;          ///< +-(46), m_n-m_p mass difference [MeV/c^2] CODATA 2018
    constexpr double m_n = m_p + delta_mn_mp;           ///< neutron mass                    [MeV/c^2]
    constexpr double neutronBetaEp = delta_mn_mp - m_e; ///< neutron beta decay endpoint     [MeV]

    constexpr double Q_e = 1.602176634e-19;             ///< exact elementary charge         [Coulomb] CODATA 2018

    constexpr double alpha = .0072973525693;            ///< +-(11), fine structure constant [ ]       CODATA 2018

    constexpr double N_A = 6.02214076e23;               ///< exact Avogadro constant         [mol^-1]  CODATA 2018

    constexpr double J_per_MeV = 1.602176634e-13;       ///< exact value of MeV in Joules              CODATA 2018
    constexpr double hbar = 1.054571817e-34;            ///< exact reduced Planck constant [J s] CODATA 2018
    constexpr double hbar_c = 197.3269804;              ///< exact hbar*c [MeV fm] CODATA 2018

    constexpr double c_mps = 299792458.;                ///< exact speed of light in vacuum  [m/s]     CODATA 2018

    constexpr double epsilon_0 = 8.8541878128e-12;      ///< +-(13), vacuum electric permittivity [F/m = C^2 / J m] CODATA 2018

    constexpr double r_e = alpha * hbar_c / m_e;        /// "classical electron radius" (alpha * hbar) / (m_e * c) [fm]

    constexpr double G_F = 1.1663787e-11;               ///< +-(6), Fermi coupling constant  [/MeV^2]  CODATA 2018

    constexpr double gamma_euler = 0.57721566490153286060651209008240243104215933593992;     ///< Euler's constant (to 50 digits)
}

#endif
