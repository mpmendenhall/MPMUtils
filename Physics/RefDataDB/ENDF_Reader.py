from ENDF_File1_Reader import *
from ENDF_File3_Reader import *
from ENDF_File6_Reader import *
from ENDF_File8_Reader import *

def load_ENDF_Section(iterlines):
    """Parse supplied ENDF lines as a file section"""

    l0 = next(iterlines)
    h = ENDF_Record(l0)

    if h.MAT <= 0 or h.MF == 0: return ENDF_CONT_Record(l0) # material, tape, or file end

    if h.MF == 1: return ENDF_File1_Sec(iterlines, l0)
    if h.MF == 3: return ENDF_File3_Sec(iterlines, l0)
    if h.MF == 6: return ENDF_File6_Sec(iterlines, l0)
    if h.MF == 8: return ENDF_File8_Sec(iterlines, l0)

    print("Unsupported file type MF = %i!"%h.MF)
    print(h)
    while h.MF: h = ENDF_Record(next(iterlines))
    return None



