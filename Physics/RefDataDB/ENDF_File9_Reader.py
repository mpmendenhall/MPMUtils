from ENDF_Base_Reader import *

class ENDF_File9_Sec(ENDF_HEAD_Record):
    """File 9 'Multiplicities for production of radioactive nuclides'"""
    def __init__(self, iterlines, l0 = None):
        if l0 is None: l0 = next(iterlines)
        super().__init__(l0)
        assert self.MF in (9,10)
        self.rectp = "File 9 'Multiplicities for production of radioactive nuclides' section %i"%self.MT
        self.rnm("L1","LIS")    # Level number of target
        self.rnm("N1","NS")     # Number of final states
        self.contents = [ENDF_Tab1(iterlines) for i in range(self.NS)]

    def __repr__(self):
        s = super().__repr__()
        for c in self.contents: s += '\n'+str(c)
        return s
