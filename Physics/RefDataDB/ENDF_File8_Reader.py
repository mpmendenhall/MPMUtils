from ENDF_Base_Reader import *

class ENDF_File8_FPY(ENDF_HEAD_Record):
    """Fission Product Yields in File 8"""
    def __init__(self, iterlines):
        super().__init__(next(iterlines))
        assert self.MF == 8 and self.MT in (454,459)
        footer = ENDF_HEAD_Record(next(iterlines))
        assert footer.rectp == "SEND"

        self.LE = self.L1 - 1
        self.contents = [ENDF_List(iterlines) for i in range(self.L1)]

        footer = ENDF_HEAD_Record(next(iterlines))
        assert footer.rectp == "SEND"

class ENDF_File8_MT457_Spectrum(ENDF_List):
    """Spectrum data structure in File 8, section 457 data"""
    def __init__(self, iterlines):
        super().__init__(iterlines)
        self.STYP = self.C2 # decay radiation type
        self.LCON = self.L1 # continuum spectrum flag. 0: no continuous given; 1: only continuous given; 2: discrete and continuous given
        self.NER = self.N2  # number of tabulated discrete energies

        self.discrete = [ENDF_List(iterlines) for i in range(self.NER)] if self.LCON != 1 else None
        self.continuous = ENDF_Tab1(iterlines) if self.LCON != 0 else None
        self.cov = ENDF_List(iterlines) if self.LCON and self.continuous.L2 else None


class ENDF_File8_Sec(ENDF_HEAD_Record):
    """File 8 'Decay and Fission Products Yields' section"""
    def __init__(self, iterlines, l0 = None):
        if l0: super().__init__(l0)
        else: super().__init__(next(iterlines))

        self.rectp = "File 8 "
        if self.MT == 454: self.rectp += "Fission Products Independent Yields"
        elif self.MT == 457: self.rectp += "Radioactive Decay"
        elif self.MT == 459: self.rectp += "Fission Products Cumulative Yields,"
        else: self.rectp += "Decay and Fission Product Yields MT-%i"%self.MT

        self.LIS  = self.L1 # state number of target
        self.LISO = self.L2 # isomeric state number of target

        self.decays = []
        if self.MT in (454,459):
            self.NS   = self.N1 # number of states for which data is provided
            self.NO   = self.N2 # 0: complete chain; 1: see MT=457 data
            if self.NO == 1: self.decays = [ENDF_CONT_Record(next(iterlines)) for i in range(self.NS)]
            else: self.decays = [ENDF_List(iterlines) for i in range(self.NS)]
        elif self.MT == 457:
            self.NST = self.N1 # nucleus stability: 0 = radioactive, 1 = stable
            self.NSP = self.N2 # number of spectrum blocks
            if self.NST: assert not self.NSP
            self.info1 = ENDF_List(iterlines)
            self.T_h = self.info1.C1    # halflife
            self.dT_h = self.info1.C2   # uncertainty on T_h
            self.info2 = ENDF_List(iterlines)
            self.decays = [ENDF_File8_MT457_Spectrum(iterlines) for i in range(self.NSP)]
        else:
            print("Unhandled data type MT=",self.MT)
            assert False

        footer = ENDF_HEAD_Record(next(iterlines))
        assert footer.rectp == "SEND"

    def printid(self):
        s = super().printid() + " target state %i (iso. %i)"%(self.LIS, self.LISO)
        if self.MT in (454,459): s += "; NS=%i%s"%(self.NS, "" if not self.NO else " +aux in MT457")
        if self.MT == 457: s += "; T_1/2 = %g ~ %g"%(self.T_h, self.dT_h)
        return s

    def __repr__(self):
        s = self.printid()
        for t in self.decays: s += '\n\t'+str(t)
        return s
