from ENDF_Base_Reader import *

class ENDF_File5_Distrib(ENDF_Tab1):
    """File 5 energy distribution distribution"""
    def __init__(self, iterlines):
        super().__init__(iterlines)
        self.rnm("L2","LF") # distribution representation flag

        if self.LF == 1:
            self.rectp += " Arbitrary tabulated distribution"
            self.distrib = ENDF_Tab2(iterlines, ENDF_Tab1)
        elif self.LF == 5:
            self.rectp += " General evaporation spectrum"
            self.theta = ENDF_Tab1(iterlines)
            self.g = ENDF_Tab1(iterlines)
        elif self.LF == 7:
            self.rectp += " Simple Maxwellian fission spectrum"
            self.theta = ENDF_Tab1(iterlines)
        elif self.LF == 9:
            self.rectp += " Evaporation spectrum"
            self.theta = ENDF_Tab1(iterlines)
        elif self.LF == 11:
            self.rectp += " Energy-dependent Watt Spectrum"
            self.a = ENDF_Tab1(iterlines)
            self.b = ENDF_Tab1(iterlines)
        elif self.LF == 12:
            self.rectp += " Energy-Dependent Madland/Nix Fission Neutron Spectrum"
            self.T_M = ENDF_Tab1(iterlines)
        else:
            print("Unhandled File 5 subsection format LF =", self.LF)
            raise NotImplementedError

class ENDF_File5_Sec(ENDF_HEAD_Record):
    """File 5 'Energy distributions of secondary particles' for neutrons and spontaneous fission"""
    def __init__(self, iterlines, l0 = None):
        if l0 is None: l0 = next(iterlines)
        super().__init__(l0)
        assert self.MF == 5
        self.rectp = "File 5 'Energy distributions of secondary particles' section %i"%self.MT
        self.rnm("N1","NK") # number of subsections

        self.distribs = [ENDF_File5_Distrib(iterlines) for i in range(self.NK)]
        footer = ENDF_HEAD_Record(next(iterlines))
        assert footer.rectp == "SEND"

    def __repr__(self):
        s = super().__repr__()
        for d in self.distribs: s += "\n"+str(d)
        return s
