from ENDF_Base_Reader import *

class ENDF_File33_NC_SSSec(ENDF_List):
    """File 33 NC-type subsubsection"""
    def __init__(self, iterlines):
        h = ENDF_CONT_Record(next(iterlines))
        super().__init__(iterlines)
        self.LTY = h.L2 # Flag for procedure to obtain covariance matrix
        assert LTY in (0,1,2,3,4)
        self.rectp += " NI"

class ENDF_File33_NI_SSSec(ENDF_List):
    """File 33 NI-type subsubsection"""
    def __init__(self, iterlines):
        super().__init__(iterlines)
        self.rectp += " NI"
        self.LB = self.L2   # flag for interpretation of contents
        self.LT = self.L1   # number of pairs in second array

class ENDF_File33_SubSec(ENDF_CONT_Record):
    """File 33 subsection [MAT,33,MT/ XMF1, XLFS1, MAT1, MT1, NC, NI]CONT"""
    def __init__(self, iterlines):
        super().__init__(next(iterlines))
        self.XMF1  = int(self.C1)   # MF for the 2nd cross section of the pair; if MF1=MF, XMF1=0.0 or blank.
        self.XLFS1 = int(self.C2)   # final excited state of the 2nd energy-dependent cross section.
        self.MAT1  = self.L1        # MAT for 2nd cross-section
        self.MT1   = self.L2        # MT for 2nd cross-section
        self.NC    = self.N1        # number of NC-type subsubsections
        self.NI    = self.N2        # number of NI-type subsubsections
        self.rectp = "File33 Subsec (%i,%i)"%(self.NC, self.NI)

        self.subNC = [ENDF_File33_NC_SSSec(iterlines) for i in range(self.NC)]
        self.subNI = [ENDF_File33_NI_SSSec(iterlines) for i in range(self.NI)]


class ENDF_File33_Sec(ENDF_HEAD_Record):
    """File 33 'Covariances of neutron cross sections'"""

    def __init__(self, iterlines, l0 = None):
        if l0 is None: l0 = next(iterlines)
        super().__init__(l0)
        assert self.MF == 33
        self.rectp = "File 33 'Covariances of neutron cross sections' section %i"%self.MT

        self.MTL = self.L2  # Nonzero: flags one component of lumped reaction; cov. not given
        self.NL  = self.N2  # number of subsections (0 in lumped MTL != 0 files)
        assert not (MTL and NL)

        self.contents = [ENDF_File33_SubSec(iterlines) for i in range(self.NL)]

        footer = ENDF_HEAD_Record(next(iterlines))
        assert footer.rectp == "SEND"

    def __repr__(self):
        s = self.printid()
        for c in self.contents: s += '\n'+str(c)
        return s
