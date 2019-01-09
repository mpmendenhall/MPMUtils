from ENDF_File9_Reader import *

class ENDF_File10_Sec(ENDF_File9_Sec):
    """File 10 'Cross sections for production of radioactive nuclides'"""
    def __init__(self, iterlines, l0 = None):
        super().__init__(iterlines,l0)
        assert self.MF == 10
        self.rectp = "Cross sections for production of radioactive nuclides %i"%self.MT
