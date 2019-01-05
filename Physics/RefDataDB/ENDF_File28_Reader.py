from ENDF_Base_Reader import *

class ENDF_File28_List(ENDF_List):
    """File 28 subsection"""
    def __init__(self, iterlines):
        super().__init__(iterlines)

class ENDF_File28_Sec(ENDF_HEAD_Record):
    """File 28 'Atomic Relaxation Data'"""

    def __init__(self, iterlines, l0 = None):
        if l0 is None: l0 = next(iterlines)
        super().__init__(l0)
        assert self.MF == 28
        assert self.MT == 533

        self.rectp = "File 28 'Atomic relaxation data'"
        self.NSS = self.N1  # number of subshell entries
        self.contents = [ENDF_File28_List(iterlines) for i in range(self.NSS)]


        footer = ENDF_HEAD_Record(next(iterlines))
        assert footer.rectp == "SEND"

    def __repr__(self):
        s = self.printid()
        for c in self.contents: s += '\n'+str(c)
        return s
