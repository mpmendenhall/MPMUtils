from ENDF_Base_Reader import *

class ENDF_File12_Sec(ENDF_HEAD_Record):
    """File 12 'Photon production multiplicities and transition probability arrays'"""

    # [MAT, 12, MT/ZA, AWR, LO, 0, NK, 0]HEAD  LO = 1
    # [MAT, 12, MT/ZA, AWR, LO, LG,NS, 0]HEAD  LO = 2

    def __init__(self, iterlines, l0 = None):
        if l0 is None: l0 = next(iterlines)
        super().__init__(l0)
        assert self.MF == 12
        assert self.N2 == 0
        self.rectp = "File 12 'Photon production multiplicities and transition probability arrays' for MT %i"%self.MT
        self.rnm("L1","LO",True) # Data format option
        assert self.LO in (0, 1, 2)

        self.contents = []

        if self.LO == 1:
            assert self.L2 == 0
            self.rnm("N1","NK",True) # number of neutron energy / gamma pairs
            if self.NK > 1: Ytotal = ENDF_Tab1(iterlines) # summed total yield
            self.contents = [ENDF_Tab1(iterlines) for i in range(self.NK)]
            if self.NK > 1: self.contents.append(Ytotal)

        if self.LO == 2:
            self.rnm("L2","LG",True)
            self.rnm("N1","NS",True)
            assert False # not yet supported



    def __repr__(self):
        s = super().__repr__()
        for c in self.contents: s += '\n'+str(c)
        return s
