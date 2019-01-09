from ENDF_File3_Reader import *

class ENDF_File27_Sec(ENDF_File3_Sec):
    """File 27 'Smooth photon interaction cross sections' section"""
    def __init__(self, iterlines, l0 = None):
        super().__init__(iterlines,l0)
        self.rectp = "File 27 'Atomic form factors or scattering functions' section %i"%self.MT
