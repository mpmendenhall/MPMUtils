#!/bin/env python3
## @file gammascatter.py Gamma-electron scattering calculations
# Michael P. Mendenhall, LLNL 2021

from math import *

mass_e = 0.51099895000    # +-(15), electron mass [MeV/c^2] CODATA 2018
alpha_fs = .0072973525693 # +-(11), fine structure constant CODATA 2018
hbar_c = 197.3269804      # exact hbar*c [MeV fm] CODATA 2018

def ge_scatter_P(Eg, costheta, m = mass_e):
    """Energy ratio E_out/E_g for gamma-electron scattering"""
    return 1/(1 + (1-costheta)*Eg/m)

def ge_scatter_costheta(Eg, Eout, m = mass_e):
    """Angle for gamma-electron scatter to output energy"""
    return 1 - (Eg - Eout)*m/(Eg * Eout)

def KN_scatter(Eg, costheta, m = mass_e):
    """Klein-Nishina gamma-electron scattering cross section d\sigma/d\Omega in [barn/steradian]"""
    P = ge_scatter_P(Eg, costheta, m)
    r_e = hbar_c / m # "reduced Compton wavelength" of electron [fm]
    k = 0.5 * alpha_fs * alpha_fs * r_e * r_e # [fm^2]
    return 0.01 * k*P*P*(P + 1./P + costheta*costheta - 1)

def KN_cross_section(Eg, m = mass_e):
    """Integrated Klein-Nishina total scattering cross section [barn]"""
    r_e = hbar_c / m # "reduced Compton wavelength" of electron [fm]
    k = 0.01 * pi * alpha_fs**2 * r_e**2 # [barn]
    a = Eg/m
    return k*(4/a**2 + 2*(a+1)/(2*a+1)**2 + (1/a - 2/a**2 -2/a**3)*log(2*a+1))

def Compton_spectrum(Eg, Ee, m = mass_e):
    """Compton scattering spectrum d\sigma/dEe in [barn/MeV]"""
    # Eg: incident gamma energy; Ee: scattered electron energy; E = Eg-Ee: scattered gamma energy
    E = Eg - Ee
    if E > Eg or E < Eg/(1 + 2*Eg/m): return 0

    cth = ge_scatter_costheta(Eg, E, m)
    P = E/Eg
    r_e = hbar_c / m # "reduced Compton wavelength" of electron [fm]
    return 0.01 * alpha_fs**2 * pi * r_e**2 * m * (P**2 + (cth**2-1)*P + 1) / (E*Eg)

if __name__ == "__main__":
    import matplotlib.pyplot as plt
    import numpy as np

    Es = np.arange(0.1, 4, .01)
    plt.plot(Es, [KN_cross_section(E) for E in Es])
    plt.ylabel('total scattering cross section, barn')
    plt.xlabel('gamma energy, MeV')
    plt.savefig("KN_xs.pdf")

    plt.figure()
    for E0 in [mass_e, 1, 2]:
        Es = np.arange(0, E0, .01)
        plt.plot(Es, [Compton_spectrum(E0, E) for E in Es])

    plt.ylabel('cross section, barn / MeV')
    plt.xlabel('scattered electron energy, MeV')
    plt.savefig("ComptonScatter.pdf")
    plt.show()
