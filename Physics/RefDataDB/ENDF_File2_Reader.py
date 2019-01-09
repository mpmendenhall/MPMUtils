from ENDF_Base_Reader import *

class ENDF_ReichMoore_Resonances(ENDF_List):
    """Specification for resolved resonances in Reich-Moore formalism"""
    def __init__(self,iterlines):
        super().__init__(iterlines)

        self.rnm("C1","AWRI")   # ratio of isotope mass to neutron
        self.rnm("C2","APL")    # l-depedent scattering radius; use APL = AP if 0
        self.rnm("L1","L")      # quantum number l
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
        super().__init__(iterlines)
        self.rnm("C1","AJ") # floating-point spin J, with sign for parity
        self.rnm("L1","INT")# interpolation scheme between cross-sections
        # AMU[XNGF]  degrees of freedom in competitive, neutron, gamma, fission distributions
        self.AMUX, self.AMUN, self.AMUG, self.AMUF = self.data[2:6]
        # data = [(ES, D, GX, GNO, GG1, GF1), ...] energy-dependent average values
        self.data = list(zip(*[iter(self.data[6:])]*6))

    def __repr__(self):
        return "[Unresolved resonances for J=%g, %i energy points]"%(self.AJ, len(self.data))

class ENDF_Unresolved_Resonances_EdepParams_L(ENDF_CONT_Record):
    """Collection of unresolved resonance data with energy-dependent parameters for a given L"""
    def __init__(self, iterlines):
        super().__init__(next(iterlines))
        self.rnm("C1","AWRI")
        self.rnm("L1","L")
        self.rnm("N1","NJS")
        self.jdat = [ENDF_Unresolved_Resonances_EdepParams_LJ(iterlines) for j in range(self.NJS)]

    def __repr__(self, longform=False):
        s = "[Unresolved resonances for l=%i, %i j's]"%(self.L, self.NJS)
        if longform:
            for j in self.jdat: s += "\n\t"+str(j)
        return s

class ENDF_File2_Range(ENDF_CONT_Record):

    LRUdescrip = {0: "radius only", 1: "resolved", 2: "unresolved" }
    LRFdescrip = {0: ""}

    def __init__(self, iterlines):
        super().__init__(next(iterlines))

        self.rnm("C1","EL")     # energy lower limit
        self.rnm("C2","EH")     # energy upper limit
        self.rnm("L1","LRU")    # resolved or unresolved parameters
        self.rnm("L2","LRF")    # representation of resonances
        self.rnm("N1","NRO")    # energy dependence of radius: 0: none; 1: from table
        self.rnm("N2","NAPS")   # interpretation of 2 radii

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

    def load_SPI_line(self, iterlines):
        c = ENDF_CONT_Record(next(iterlines))
        self.SPI    = c.C1  # spin I of target nucleus
        self.AP     = c.C2  # scattering radius in 10^-12 cm
        self.hdr_L1 = c.L1
        self.hdr_L2 = c.L2
        self.NLS    = c.N1  # number of L-values
        self.hdr_N2 = c.N2

    def __repr__(self, longform = False):
        s = "[Resonance range %g -- %g eV: %s, %s for %i l's]"
        s = s%(self.EL, self.EH, self.LRUdescrip[self.LRU], self.LRFdescrip.get(self.LRF, "LRF=%i"%self.LRF), self.NLS)
        if longform:
            if self.LRU in [1,2]:
                for l in self.ldata: s += "\n"+indent(l.__repr__(True),"\t")
        return s


class ENDF_File2_Sec(ENDF_HEAD_Record):
    """File 2 'Resonance Parameters' section"""
    def __init__(self, iterlines, l0 = None):
        if l0 is None: l0 = next(iterlines)
        super().__init__(l0)
        self.NIS = self.N1 # NIS   number of isotopes in material
        self.rectp = "Resonance Parameters"

        c = ENDF_CONT_Record(next(iterlines))
        self.ZAI = int(c.C1) # isotope Z,A
        self.ABN = int(c.C2) # isotope abundance in material, number fraction
        self.LFW = c.L2      # whether average fission widths given
        self.NER = c.N1      # number of ranges

        self.ranges = [ENDF_File2_Range(iterlines) for ri in range(self.NER)]
        footer = ENDF_HEAD_Record(next(iterlines))
        assert footer.rectp == "SEND"

    def __repr__(self):
        s = self.printid()
        for r in self.ranges: s += "\n\t"+str(r)
        return s
