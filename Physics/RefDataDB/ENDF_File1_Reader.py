from ENDF_Base_Reader import *

class ENDF_File1_SecHead(ENDF_HEAD_Record):
    """File 1 'General Information' section"""
    def __init__(self, l0):
        super().__init__(l0)
        assert self.MF == 1
        self.rectp = "File 1 'General Information' section %i"%self.MT

class ENDF_File1_Sec451(ENDF_File1_SecHead):
    """File 1 'General Information' section"""
    def __init__(self, iterlines, l0 = None):
        if l0: super().__init__(l0)
        else: super().__init__(next(iterlines))
        assert self.MT == 451

        self.LRP = self.L1
        self.LFI = self.L2
        self.NLIB = self.N1
        self.NMOD = self.N2
        c = ENDF_CONT_Record(next(iterlines))
        c = ENDF_CONT_Record(next(iterlines))
        c = ENDF_CONT_Record(next(iterlines))
        self.NWD = c.N1
        self.NXC = c.N2
        self.txt = [ENDF_Record(next(iterlines)).TEXT for i in range(self.NWD)]
        self.xc  = [ENDF_CONT_Record(next(iterlines)) for i in range(self.NXC)]

        footer = ENDF_HEAD_Record(next(iterlines))
        assert footer.rectp == "SEND"

    def __repr__(self):
        s = self.printid()
        for t in self.txt: s += '\n# '+t
        return s


def ENDF_File1_Sec(iterlines, l0 = None):
    if l0 is None: l0 = next(iterlines)
    h = ENDF_HEAD_Record(l0)
    assert h.MF == 1

    f = None
    if h.MT == 451: f = ENDF_File1_Sec451(iterlines, l0)

    assert f is not None
    return f
