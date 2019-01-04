from ENDF_Base_Reader import *

class ENDF_File4_LTT1(ENDF_Tab2):
    """File 4 Legendre Polynomials angular distribution"""
    def __init__(self, iterlines):
        super().__init__(iterlines, ENDF_List)
        self.rectp += " Legendre coefficients"

class ENDF_File4_LTT2(ENDF_Tab2):
    """File 4 probability angular distribution"""
    def __init__(self, iterlines):
        super().__init__(iterlines, ENDF_Tab1)
        self.rectp += " Probability distribution"

class ENDF_File4_Sec(ENDF_HEAD_Record):
    """File 4 'Secondary particle angular distributions' for neutron interactions"""
    def __init__(self, iterlines, l0 = None):
        if l0 is None: l0 = next(iterlines)
        super().__init__(l0)
        assert self.MF == 4
        self.rectp = "File 4 'Secondary particle angular distributions' section %i"%self.MT

        # representation flag
        # 0: all isotropic
        # 1: Legendre expansions a_l(E)
        # 2: probability f(mu, E)
        # 3: Legendre at low energy, tabulated at high energy
        self.LTT = self.L2
        assert self.LTT in (0,1,2,3)

        c = ENDF_CONT_Record(next(iterlines))
        self.LI = c.L1      # all isotropic flag
        assert bool(self.LI) != bool(self.LTT)
        self.LCT = c.L2     # reference frame flag, 1: lab, 2: CM
        assert self.LCT in (1,2)
        self.NM = c.N2      # maximum order Legendre polynomial
        assert not (self.LTT != 3 and self.NM)

        self.distribL = self.distribP = None
        if self.LTT == 1 or self.LTT == 3: self.distribL = ENDF_File4_LTT1(iterlines)
        if self.LTT == 2 or self.LTT == 3: self.distribP = ENDF_File4_LTT2(iterlines)

        footer = ENDF_HEAD_Record(next(iterlines))
        assert footer.rectp == "SEND"

    def __repr__(self):
        s = self.printid()
        if self.distribL is not None: s += "\n"+str(self.distribL)
        if self.distribP is not None: s += "\n"+str(self.distribP)
        return s
