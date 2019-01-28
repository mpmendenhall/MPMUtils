from ENDF_File1_Reader import *
from ENDF_File2_Reader import *
from ENDF_File3_Reader import *
from ENDF_File4_Reader import *
from ENDF_File5_Reader import *
from ENDF_File6_Reader import *
from ENDF_File7_Reader import *
from ENDF_File8_Reader import *
from ENDF_File9_Reader import *
from ENDF_File10_Reader import *
from ENDF_File23_Reader import *
from ENDF_File26_Reader import *
from ENDF_File27_Reader import *
from ENDF_File28_Reader import *
from ENDF_File33_Reader import *

class ENDF_TapeHeader(ENDF_Record):
    """Tape header record type"""
    def __init__(self,l0):
        super().__init__(l0)
        assert self.MF == self.MT == 0 and self.MAT == 1
        self.rectp = "Tape Header"
        self.endlvl = -4


def load_ENDF_Section(iterlines):
    """Parse supplied ENDF lines as a file section"""

    l0 = next(iterlines)
    h = ENDF_Record(l0)

    if h.MAT == 1 and h.MF == h.MT == 0: return ENDF_TapeHeader(l0)
    if h.MAT <= 0 or h.MF == 0: return ENDF_CONT_Record(l0) # material, tape, or file end

    if   h.MF == 1:  return ENDF_File1_Sec(iterlines, l0)
    elif h.MF == 2:  return ENDF_File2_Sec(iterlines, l0)
    elif h.MF == 3:  return ENDF_File3_Sec(iterlines, l0)
    elif h.MF == 4:  return ENDF_File4_Sec(iterlines, l0)
    elif h.MF == 5:  return ENDF_File5_Sec(iterlines, l0)
    elif h.MF == 6:  return ENDF_File6_Sec(iterlines, l0)
    elif h.MF == 7:  return ENDF_File7_Sec(iterlines, l0)
    elif h.MF == 8:  return ENDF_File8_Sec(iterlines, l0)
    elif h.MF == 9:  return ENDF_File9_Sec(iterlines, l0)
    elif h.MF == 10: return ENDF_File10_Sec(iterlines, l0)
    elif h.MF == 23: return ENDF_File23_Sec(iterlines, l0)
    elif h.MF == 26: return ENDF_File26_Sec(iterlines, l0)
    elif h.MF == 27: return ENDF_File27_Sec(iterlines, l0)
    elif h.MF == 28: return ENDF_File28_Sec(iterlines, l0)
    elif h.MF == 33: return ENDF_File33_Sec(iterlines, l0)

    print("Unsupported file type MF = %i!"%h.MF)
    print(h)
    while h.MF: h = ENDF_Record(next(iterlines))
    return None
