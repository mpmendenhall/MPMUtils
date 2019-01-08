from ENDF_Base_Reader import *

class ENDF_File7_TList(ENDF_List):
    def __init__(self, iterlines):
        super().__init__(iterlines)
        self.T = self.C1    # temperature
        self.beta = self.C2 # energy transfer, (E'-E)/kT

    def printid(self):
        return self.rectp + " \tT = %g"%self.T

class ENDF_File7_TTable(ENDF_Tab1):
    """Table with additional temperature columns"""
    def __init__(self, iterlines):
        super().__init__(iterlines)
        self.T0 = self.C1   # temperature
        self.beta = self.C2 # energy transfer, (E'-E)/kT
        self.LT = self.L1   # number of temperature-dependent columns
        self.cols = [ENDF_File7_TList(iterlines) for i in range(self.LT)]

    def printid(self):
        return self.rectp + " T0 = %g, beta = %g"%(self.T0, self.beta)

    def __repr__(self):
        s = self.printid()
        for c in self.cols: s += '\n\t* '+str(c)
        return s

class ENDF_File7_IncoherentInelastic(ENDF_Tab2):
    """Incoherent inelastic scattering table"""
    def __init__(self, iterlines):
        self.B = ENDF_List(iterlines) # scattering parameters array B
        super().__init__(iterlines, ENDF_File7_TTable)
        nTEff = 1 + sum([self.B.data[6+6*i] == 0 for i in range(len(self.B.data)//6-1)])
        self.Teffs = [ENDF_Tab1(iterlines) for i in range(nTEff)]
        self.rectp += " IncoherentInelastic"

class ENDF_File7_IncoherentElastic(ENDF_Tab1):
    """Incoherent elastic scattering table"""
    def __init__(self, iterlines):
        super().__init__(iterlines)
        self.rectp += " IncoherentElastic"

class ENDF_File7_Sec(ENDF_HEAD_Record):
    """File 7 'Thermal Neutron Scattering Law Data' section"""
    def __init__(self, iterlines, l0 = None):
        if l0: super().__init__(l0)
        else: super().__init__(next(iterlines))

        self.rectp = "File 7/%i"%(self.MT)
        if self.MT == 2: self.rectp += " Thermal n elastic"
        elif self.MT == 4: self.rectp += " Thermal n inelastic"
        else: assert False

        self.LTHR = self.L1 # interpretation flag; 1: elastic

        self.tbl = None
        if self.LTHR == 0:
            assert self.N2 == 0
            self.tbl = ENDF_File7_IncoherentInelastic(iterlines)
        elif self.LTHR == 1:
            assert self.L2 == self.N1 == self.N2 == 0
            self.tbl = ENDF_File7_TTable(iterlines)
        elif self.LTHR == 2:
            assert self.L2 == self.N1 == self.N2 == 0
            self.tbl = ENDF_File7_IncoherentElastic(iterlines)
        else:
            print(self)
            assert False

        footer = ENDF_HEAD_Record(next(iterlines))
        assert footer.rectp == "SEND"

    def printid(self):
        return super().printid() + " LTHR %i"%(self.LTHR)

    def __repr__(self):
        s = self.printid()
        if self.tbl is not None: s += '\n'+str(self.tbl)
        return s
