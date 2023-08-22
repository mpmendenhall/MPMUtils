from ENDF_Base_Reader import *

class ENDF_File14_Sec(ENDF_HEAD_Record):
    """File 14 'Photon angular distributions'"""

    # [MAT, 14, MT/ ZA, AWR, LI, 0, NK, 0]HEAD         LI=1 (isotropic)

    # [MAT, 14, MT/ ZA, AWR, LI, LTT, NK, NI]HEAD      LI=0 (anisotropic), with LTT=1 (Legendre) or LTT=2 (tabular)
    #   <subsection for k=1>
    #   <subsection for k=2>
    # ------------------------
    #   <subsection for k=NK>
    # [MAT, 14, 0/ 0.0, 0.0, 0, 0, 0, 0] SEND

    def __init__(self, iterlines, l0 = None):
        if l0 is None: l0 = next(iterlines)
        super().__init__(l0)
        assert self.MF == 14
        self.rectp = "File 14 'Photon angular distributions' for MT %i"%self.MT
        self.rnm("L1","LI",True) # Data format option
        self.rnm("N1","NK",True)
        assert self.LI in (0,1)

        if self.LI == 1:
            assert self.L2 == 0
            assert self.N2 == 0

        if self.LI == 0:
            self.rnm("L2","LTT",True)
            assert self.LTT in (0,1,2)  # 0 is undefined...
            self.rnm("N2","NI",True)
            assert self.NI <= self.NK
            self.contents = [ENDF_CONT_Record(next(iterlines)) for i in range(self.NI)]
            if self.LTT == 1:
                assert False
            if self.LTT == 2:
                assert False
