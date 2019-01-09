from ENDF_Base_Reader import *

class ENDF_File3_Sec(ENDF_Tab1):
    """File 3 'reaction cross sections' section"""
    def __init__(self, iterlines, l0 = None):
        if l0 is None: l0 = next(iterlines)
        h = ENDF_HEAD_Record(l0)
        assert h.MF in (3,23,27)
        super().__init__(iterlines)
        self.ZA, self.Z, self.A, self.AWR = h.ZA, h.Z, h.A, h.AWR
        assert self.MF == h.MF
        self.rectp = "File 3 'Reaction cross sections' section %i"%self.MT
        self.rnm("C1","QM") # mass-difference Q value [eV]
        self.rnm("C2","QI") # QM for residual nucleus ground state
        self.rnm("L2","LR") # complex breakup flag: additional particles may be emitted than MT specifies
        self.xu = "energy [eV]"
        self.yu = "sigma [bn]"

        footer = ENDF_HEAD_Record(next(iterlines))
        assert footer.rectp == "SEND"

    def printid(self):
        return super().printid() + ", QM=%g, Qi=%g; LR=%i"%(self.QM, self.QI, self.LR)
