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

class ENDF_File8_Sec(ENDF_HEAD_Record):
    """File 8 'Decay and Fission Products Yields' section"""
    def __init__(self, iterlines, l0 = None):
        if l0: super().__init__(l0)
        else: super().__init__(next(iterlines))

        self.rectp = "File 8 "
        if self.MT == 454: self.rectp += "Fission Products Independent Yields"
        elif self.MT == 459: self.rectp += "Fission Products Cumulative Yields,"
        elif self.MT == 459: self.rectp += "Radioactive Decay"
        else: self.rectp += "Decay and Fission Product Yields MT-%i"%self.MT

        self.LIS  = self.N1 # state number of target
        self.LISO = self.N2 # isomeric state number of target
        self.NS   = self.L1 # number of states for which data is provided
        self.NO   = self.L2 # 0: complete chain; 1: see MT=457 data

        self.decays = []
        if self.NO == 1: self.decays = [ENDF_CONT_Record(next(iterlines)) for i in range(self.NS)]
        elif self.MT in (454,459): self.decays = [ENDF_List(iterlines) for i in range(self.NS)]
        else:
            print("Unhandled data type MT=",self.MT)
            assert False

        footer = ENDF_HEAD_Record(next(iterlines))
        assert footer.rectp == "SEND"

    def printid(self):
        return super().printid() + " target state %i(%i); NS=%i%s"%(self.LIS, self.LISO, self.NS, "" if not self.NO else " +aux in MT457")

    def __repr__(self):
        s = self.printid()
        for t in self.decays: s += '\n'+str(t)
        return s
