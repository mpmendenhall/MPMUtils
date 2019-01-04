from ENDF_Base_Reader import *

class ENDF_File10_Sec(ENDF_HEAD_Record):
    """File 10 'Cross sections for production of radioactive nuclides'"""
    def __init__(self, iterlines, l0 = None):
        if l0 is None: l0 = next(iterlines)
        super().__init__(l0)
        assert self.MF == 10
        self.rectp = "Cross sections for production of radioactive nuclides %i"%self.MT
        self.LIS = self.L1  # Level number of target
        self.NS  = self.N1  # Number of final states
        self.contents = [ENDF_Tab1(iterlines) for i in range(self.NS)]

    def __repr__(self):
        s = super().__repr__()
        for c in self.contents: s += '\n'+str(c)
        return s
