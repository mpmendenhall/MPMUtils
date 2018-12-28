# TODO not up-to-date

from ENDF_Base_Reader import *

class ENDF_ReichMoore_Resonances(ENDF_List):
    """Specification for resolved resonances in Reich-Moore formalism"""
    def __init__(self,iterlines):
        ENDF_List.__init__(self,iterlines)
        # AWRI  ratio of isotope mass to neutron
        # APL   l-depedent scattering radius; use APL=AP if 0
        # L     quantum number l
        self.AWRI = self.header.C1
        self.APL = self.header.C2
        self.L = self.header.L1
        self.data = list(zip(*[iter(self.data)]*6))

    def __repr__(self, longform=False):
        s = "[Reich-Moore resonances (%i) for l=%i]"%(len(self.data), self.L)
        if longform:
            for d in self.data[:10]: s += "\n\t"+str(d)
            if len(self.data) > 10: s += "\n\t... and %i more"%(len(self.data)-10)
        return s

class ENDF_Unresolved_Resonances_EdepParams_LJ(ENDF_List):
    """Specification for unresolved resonances with energy-dependent parameters for L,J"""
    def __init__(self, iterlines):
        ENDF_List.__init__(self,iterlines)
        # AJ    floating-point spin J, with sign for parity
        # INT   interpolation scheme between cross-sections
        # AMU[XNGF]  degrees of freedom in competitive, neutron, gamma, fission distributions
        self.AJ = self.header.C1
        self.INT = self.header.L1
        self.AMUX, self.AMUN, self.AMUG, self.AMUF = self.data[2:6]
        # data = [(ES, D, GX, GNO, GG1, GF1), ...] energy-dependent average values
        self.data = list(zip(*[iter(self.data[6:])]*6))

    def __repr__(self):
        return "[Unresolved resonances for j=%g, %i energy points]"%(self.AJ, len(self.data))

class ENDF_Unresolved_Resonances_EdepParams_L:
    """Collection of unresolved resonance data with energy-dependent parameters for a given L"""
    def __init__(self, iterlines):
        self.AWRI, u1, self.L, u2, self.NJS, u3 = ENDF_CONT_Record.parse(next(iterlines))
        self.jdat = [ENDF_Unresolved_Resonances_EdepParams_LJ(iterlines) for j in range(self.NJS)]

    def __repr__(self, longform=False):
        s = "[Unresolved resonances for l=%i, %i j's]"%(self.L, self.NJS)
        if longform:
            for j in self.jdat: s += "\n\t"+str(j)
        return s

class ENDF_File2_Range:

    LRUdescrip = {0: "radius only", 1: "resolved", 2: "unresolved" }
    LRFdescrip = {0: ""}

    def __init__(self, iterlines, LFW):
        self.LFW = LFW

        # EL    energy lower limit
        # EH    energy upper limit
        # LRU   resolved or unresolved parameters
        # LRF   representation of resonances
        # NRO   energy dependence of radius: 0: none; 1: from table
        # NAPS  interpretation of 2 radii
        self.EL, self.EH, self.LRU, self.LRF, self.NRO, self.NAPS = ENDF_CONT_Record.parse(next(iterlines))

        if self.LRU == 0: # only scattering radius given
            self.load_SPI_line(iterlines)
            assert self.NLS == 0

        elif self.LRU == 1: # resolved resonance parameters
            self.LRFdescrip = {1: "SLBW", 2: "MLBW", 3: "Reich-Moore", 4: "Adler-Adler", 7: "R-Matrix Limited"}

            if self.LRF in (1,2,3):
                self.ertable = ENDF_Tab1(iterlines) if self.NRO else None # optional energy radius table

                self.load_SPI_line(iterlines)
                # LAD   whether to use parameters to compute angular distributions (0/1)
                self.LAD = self.hdr_L1
                if self.LRF == 3:
                    # NLSC  number of l's to converge elastic angular distributions, >= NLS
                    self.NLSC = self.hdr_N2
                else: raise NotImplementedError

                # parameter list for each l
                self.ldata = [{1:ENDF_List, 2:ENDF_List, 3:ENDF_ReichMoore_Resonances}[self.LRF](iterlines) for l in range(self.NLS)]

            else:
                print("LRF = %i unimplemented"%self.LRF)
                raise NotImplementedError

        elif self.LRU == 2: # unresolved resonance parameters, described by SLBW
            self.LRFdescrip = {1: "energy-dependent widths", 2: "energy-dependent parameters"}

            #if self.LRF == 1: # no or only fission widths given with energy dependence
            #    pass

            if self.LRF == 2: # all parameters may be energy dependent

                self.load_SPI_line(iterlines)
                # LSSF: interpretation of File 3 cross sections
                self.LSSF = self.hdr_L1
                assert self.hdr_L2 == 0 and self.hdr_N2 == 0
                self.ldata = [ENDF_Unresolved_Resonances_EdepParams_L(iterlines) for l in range(self.NLS)]

            else:
                print("LRF = %i unimplemented"%self.LRF)
                raise NotImplementedError
        else:
            print("LRU = %i unimplemented"%self.LRU)
            raise NotImplementedError

    def load_SPI_line(self,iterlines):
        # SPI   spin I of target nucleus
        # AP    scattering radius in 10^-12 cm
        # NLS   number of L-values
        self.SPI, self.AP, self.hdr_L1, self.hdr_L2, self.NLS, self.hdr_N2 = ENDF_CONT_Record.parse(next(iterlines))

    def __repr__(self, longform = False):
        s = "[Resonance range %g -- %g eV: %s, %s for %i l's]"
        s = s%(self.EL, self.EH, self.LRUdescrip[self.LRU], self.LRFdescrip.get(self.LRF, "LRF=%i"%self.LRF), self.NLS)
        if longform:
            if self.LRU in [1,2]:
                for l in self.ldata: s += "\n"+indent(l.__repr__(True),"\t")
        return s


class ENDF_File2:
    """File 2 'Resonance Parameters'"""
    def __init__(self, iterlines):
        # ZA
        # AWR
        # NIS   number of isotopes in material
        self.ZA, self.AWR, u1, u2, self.NIS, u3 = ENDF_CONT_Record.parse(next(iterlines))
        # ZAI   isotope Z,A
        # ABN   isotope abundance in material, number fraction
        # LFW   average fission widths given? 0: no, 1: yes
        # NER   number of ranges
        self.ZAI, self.ABN, u1, self.LFW, self.NER, u2 = ENDF_CONT_Record.parse(next(iterlines))

        self.ranges = [ENDF_File2_Range(iterlines, self.LFW) for ri in range(self.NER)]
        assert skip_to_section_end(iterlines) == 1
        skip_to_file_end(iterlines)
        #assert skip_to_file_end(iterlines) == 1

    def __repr__(self, longform = False):
        s = "[File 2: %i resonance parameter ranges]"%len(self.ranges)
        if longform:
            for r in self.ranges: s += "\n"+indent(r.__repr__(True),"\t")
        return s
