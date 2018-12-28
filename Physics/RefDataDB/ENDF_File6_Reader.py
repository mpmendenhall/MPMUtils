from ENDF_Base_Reader import *

class ENDF_File6_LAW1(ENDF_Tab2):
    """LAW=1 angular distribution"""
    def __init__(self, iterlines):
        super().__init__(iterlines)
        self.rectp += " E/angle Distribution"

        # Angular distribution, in:
        # (1) Legendre Coefficients,
        # (2) Kalbach-Mann systematics,
        # (11)-(15) interpolation tables
        self.LANG = self.L1
        # Interpolation scheme for secondary energy:
        # (1) histogram,
        # (2) linear-linear, etc.
        self.LEP = self.L2
        # TAB2 interpolation parameters
        self.NE = self.N2

        self.entries = [ENDF_List(iterlines) for n in range(self.NE)]

    def printid(self):
        return super().printid() + ' LANG=%i, LEP=%i, NE=%i'%(self.LANG, self.LEP, self.NE)


class ENDF_File6_Tab1(ENDF_Tab1):
    """Table in File 6 data"""
    def __init__(self, iterlines):
        super().__init__(iterlines)
        assert self.MF == 6
        self.rectp += " Products Distribution"
        self.ZAP = self.C1 # product 1000*Z + A; =0 for photon
        self.AWP = self.C2 # product mass, neutron units; or, energy of primary photon for ZAP=0
        self.LIP = self.L1 # product modifier flag
        self.LAW = self.L2 # distribution type
        self.xu = "product energy [eV]"
        self.yu = "product yield multiplicity [1]"

        self.distrib = None
        # undefined distribution
        if not self.LAW: return

        # Continuum Energy-Angle Distributions
        if self.LAW == 1:
            self.distrib = ENDF_File6_LAW1(iterlines)
            return

        print("Unknown LAW", self.LAW)
        assert False

    def printid(self):
        return super().printid() + " ZAP=%g, AWP=%g, LIP=%i, LAW=%i"%(self.ZAP, self.AWP, self.LIP, self.LAW)

    def __repr__(self):
        s = super().__repr__()
        if self.distrib is not None: s += '\n'+str(self.distrib)
        return s

class ENDF_File6_Sec(ENDF_HEAD_Record):
    """File 6 'Product Energy-Angle Distributions' section"""
    def __init__(self, iterlines, l0 = None):
        if l0: super().__init__(l0)
        else: super().__init__(next(iterlines))
        assert self.MF == 6
        self.rectp = "File 6 'Product Energy-Angle Distributions' section %i"%self.MT
        self.LCT = self.N2 # reference frame for secondary energy, angle specification
        self.NK = self.N1  # number of subsections

        self.sections = [ENDF_File6_Tab1(iterlines) for i in range(self.NK)]
        self.footer = ENDF_HEAD_Record(next(iterlines))
        assert self.footer.rectp == "SEND"

    def __repr__(self):
        s = self.printid() +', %g AMU; %i products in frame %i'%(self.AWR, self.NK, self.LCT)
        for t in self.sections: s += '\n'+str(t)
        return s
