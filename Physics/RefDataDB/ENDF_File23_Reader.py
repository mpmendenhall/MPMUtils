from ENDF_File3_Reader import *

class ENDF_File23_Sec(ENDF_File3_Sec):
    """File 23 'Smooth photon interaction cross sections' section"""
    def __init__(self, iterlines, l0 = None):
        super().__init__(iterlines,l0)
        self.rectp = "File 23 'Smooth photon interaction cross sections' section %i"%self.MT
