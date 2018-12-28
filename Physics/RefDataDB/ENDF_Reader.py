from ENDF_File3_Reader import *
from ENDF_File6_Reader import *

def load_ENDF_Section(iterlines):
    """Parse supplied ENDF lines as a file section"""

    l0 = next(iterlines)
    h = ENDF_CONT_Record(l0)
    if h.MF == 3: return ENDF_File3_Sec(iterlines, l0)
    if h.MF == 6: return ENDF_File6_Sec(iterlines, l0)

    print("Unsupported section type!")
    print(h)
    return None



