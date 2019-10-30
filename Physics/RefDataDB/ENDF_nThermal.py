#! /bin/env python3
"""@package ENDF_nThermal
Extract thermal neutron capture cross-section from ENDF6"""

from ENDF6_DB import *
from math import *

m_n = 939.5654133e6 # CODATA 2014 neutron mass, eV
k_B = 8.6173303e-5  # CODATA 2014 Boltmann constant, eV/K

class ncapt_thermal:
    """1/v-region thermal neutron capture cross section estimate from ENDF data"""
    def __init__(self, EDB, Z, A, T = 300):

        self.Z = Z
        self.A = A

        # neutron characteristic velocity at temperature [c]
        self.vc = sqrt(pi*k_B*T/(2*m_n))
        self.Ec = 0.25*pi*k_B*T

        # total interaction cross section
        sTot = EDB.find_sections({"A": A, "Z": Z, "MF": 3, "MT": 1})
        if not sTot:
            self.s = None
            return
        sTot = EDB.get_section(sTot[0])

        #E0 = sTot.xs[0]
        #s0 = sTot.ys[0]
        E0 = self.Ec
        s0 = sTot(E0)

        # neutron velocity at measured point [c]
        v0 = sqrt(2*E0/m_n)

        nel0 = 0;

        if not (Z==3 and A==6):
            # MT102 radiative capture for non-6Li?
            sRC = EDB.find_sections({"A": A, "Z": Z, "MF": 3, "MT": 102})
            nel0 = EDB.get_section(sRC[0])(E0) if sRC else None
        if not nel0:
            # Non-elastic directly available?
            sNEl = EDB.find_sections({"A": A, "Z": Z, "MF": 3, "MT": 3})
            nel0 = EDB.get_section(sNEl[0])(E0) if sNEl else None

        if nel0:
            #print("\tof which", nel0, "is inelastic")
            s0 = nel0
        else:
            # subtract elastic scattering if available
            sEl = EDB.find_sections({"A": A, "Z": Z, "MF": 3, "MT": 2})
            el0 = EDB.get_section(sEl[0])(E0) if sEl else None
            if el0:
                #print("\tof which", el0, "is elastic scattering")
                s0 -= el0

        # thermal-averaged cross section
        self.s = s0*v0/self.vc

    def display(self):
        print("Z =", self.Z, "\tA =", self.A)
        print("v * sigma =",  self.vc * 299792458, "[m/s] *", self.s, "[b]")

if __name__ == "__main__":
    EDB = ENDFDB()
    ncapt_thermal(EDB, 1, 1).display()
    ncapt_thermal(EDB, 3, 6).display()
    ncapt_thermal(EDB, 8, 16).display()
    ncapt_thermal(EDB, 17, 35).display()
