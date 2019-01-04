from ENDF_Base_Reader import *


class ENDF_File3_Tab1(ENDF_Tab1):
    """Table in File 3 data"""
    def __init__(self, iterlines):
        super().__init__(iterlines)
        assert self.MF == 3

        self.rectp += " Cross Sections"
        self.QM = self.C1 # mass-difference Q value [eV]
        self.QI = self.C2 # QM for residual nucleus ground state
        self.LR = self.L2 # complex breakup flag: additional particles may be emitted than MT specifies
        self.xu = "energy [eV]"
        self.yu = "sigma [bn]"

    def printid(self):
        return super().printid() + " QM=%g, Qi=%g; LR=%i"%(self.QM, self.QI, self.LR)


class ENDF_File3_Sec(ENDF_HEAD_Record):
    """File 3 'reaction cross sections' section"""
    def __init__(self, iterlines, l0 = None):
        if l0: super().__init__(l0)
        else: super().__init__(next(iterlines))
        assert self.MF == 3
        self.rectp = "File 3 'Reaction cross sections' section %i"%self.MT
        self.AWR = self.C2
        self.tbl = ENDF_File3_Tab1(iterlines)
        footer = ENDF_HEAD_Record(next(iterlines))
        assert footer.rectp == "SEND"

    def printid(self):
        return super().printid() + ', %g AMU'%(self.AWR)

    def __repr__(self):
        return self.printid() + '\n' + str(self.tbl)
