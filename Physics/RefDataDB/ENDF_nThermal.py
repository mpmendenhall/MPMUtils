#! /bin/env python3
"""@package ENDF_nThermal
Extract thermal neutron capture cross-section from ENDF6"""

from ENDF6_DB import *
from math import *

m_n = 939.5654133e6 # CODATA 2014 neutron mass, eV
k_B = 8.6173303e-5  # CODATA 2014 Boltmann constant, eV/K

def ncapt_thermal(EDB, Z, A, T = 300):
    """1/v-region thermal neutron capture cross section estimate from ENDF data"""

    print("Z =", Z, "\tA =", A)

    # neutron characteristic velocity at temperature [c]
    vc = sqrt(pi*k_B*T/(2*m_n))
    Ec = 0.25*pi*k_B*T

    # total interaction cross section
    sTot = EDB.find_sections({"A": A, "Z": Z, "MF": 3, "MT": 1})
    if not sTot: return None
    sTot = EDB.get_section(sTot[0])
    #print(sTot)

    #E0 = sTot.xs[0]
    #s0 = sTot.ys[0]
    E0 = Ec
    s0 = sTot(E0)

    # neutron velocity at measured point [c]
    v0 = sqrt(2*E0/m_n)

    # subtract any elastic scattering component
    sEl = EDB.find_sections({"A": A, "Z": Z, "MF": 3, "MT": 2})
    el0 = EDB.get_section(sEl[0])(E0) if sEl else 0

    print("Total cross section", s0, "at E =", E0, "MeV")
    if el0:
        print("\tof which", el0, "is elastic scattering")
        s0 -= el0

    # thermal-averaged cross section
    s = s0*v0/vc

    print("v * sigma =",  vc * 299792458, "[m/s] *", s, "[bn]")
    return s

if __name__ == "__main__":
    EDB = ENDFDB()
    ncapt_thermal(EDB, 1, 1)
    ncapt_thermal(EDB, 3, 6)
    ncapt_thermal(EDB, 17, 35)
