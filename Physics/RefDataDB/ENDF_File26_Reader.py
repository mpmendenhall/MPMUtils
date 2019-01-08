from ENDF_File6_Reader import *

class ENDF_File26_Sec(ENDF_File6_Sec):
    """File 26 'Secondary distributions for photo- and electro-atomic data' section"""
    def __init__(self, iterlines, l0 = None):
        super().__init__(iterlines,l0)
        self.rectp = "File %i 'Secondary distributions for photo- and electro-atomic data' section %i"%(self.MF, self.MT)
