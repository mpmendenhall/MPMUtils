from ENDF_Base_Reader import *

class ENDF_File6_LAW1_List(ENDF_List):
    """List entry in LAW=1 File 6 table"""
    def __init__(self, iterlines):
        super().__init__(iterlines)
        self.E1 = self.C2       # incident energy
        self.NEP =self.N2       # number of secondary energies listed
        self.ND = self.L1       # number of discrete energies listed
        self.NA = self.L2       # number of angular parameters

        nper = self.NA + 2
        self.contents = [(self.data[i*nper], self.data[i*nper + 1: (i+1)*nper]) for i in range(self.NEP)]

    def __repr__(self):
        s = self.printid() +'=E1,\t%i ND, %i NA,\t%i NEP'%(self.ND, self.NA, self.NEP)
        #for c in self.contents: s += '\n\t%g MeV out:\t'%(1e-6*c[0]) + str([1e6*x for x in c[1]])
        return s

class ENDF_File6_LAW1(ENDF_Tab2):
    """LAW=1 angular distribution [MAT, 6, MT/ 0.0, 0.0, LANG, LEP, NR, NE/ E_int]TAB2"""
    def __init__(self, iterlines):
        super().__init__(iterlines, ENDF_File6_LAW1_List)
        self.rectp += " Continuum E/angle Distribution"

        # Angular distribution, in:
        # (1) Legendre Coefficients,
        # (2) Kalbach-Mann systematics,
        # (11)-(15) interpolation tables
        self.LANG = self.L1
        # Interpolation scheme for secondary energy:
        # (1) histogram,
        # (2) linear-linear, etc.
        self.LEP = self.L2
        # number of sub-entries (one per incident energy)
        self.NE = self.NZ

    def printid(self):
        return super().printid() + ' LANG=%i, LEP=%i, NE=%i'%(self.LANG, self.LEP, self.NE)


class ENDF_File6_LAW2_List(ENDF_List):
    """List entry in LAW=2 File 6 table [MAT, 6, MT/ 0.0, E_1 ,LANG, 0, NW, NL/ A_l(E)]LIST"""
    def __init__(self, iterlines):
        super().__init__(iterlines)
        self.E1 = self.C2       # incident energy
        self.LANG = self.L1     # angular distribution type
        self.NL = self.N2       # LANG=0: highest Legendre order used; else number of cosines tabulated.
        self.rectp = "LIST.LAW2"

    def __repr__(self):
        return self.printid() +'; E1=%g, LANG=%i, NL=%i'%(self.E1, self.LANG, self.NL)

class ENDF_File6_LAW2(ENDF_Tab2):
    """LAW=2 angular distribution"""
    def __init__(self, iterlines, ENDF_File6_LAW2_List):
        super().__init__(iterlines)
        self.rectp += " Discrete 2-body scattering"
        # number of sub-entries
        self.NE = self.NZ

class ENDF_File6_Tab1(ENDF_Tab1):
    """Table in File 6 data"""
    def __init__(self, iterlines):
        super().__init__(iterlines)
        assert self.MF == 6
        self.rectp += " Products Distribution"
        self.ZAP = self.ZA # product 1000*Z + A
        self.AWP = self.C2 # product mass, neutron units; or, energy of primary photon for ZAP=0
        self.LIP = self.L1 # product modifier flag
        self.LAW = self.L2 # distribution type
        self.xu = "product energy [eV]"
        self.yu = "product yield multiplicity [1]"

        self.distrib = None
        # undefined distribution
        if not self.LAW: return
        if self.LAW == 3: return    # isotropic in CM frame
        if self.LAW == 4: return    # discrete 2-body recoils

        # Continuum Energy-Angle Distributions
        if self.LAW == 1: self.distrib = ENDF_File6_LAW1(iterlines)
        if self.LAW == 2: self.distrib = ENDF_File6_LAW2(iterlines)
        if self.distrib: return

        print("Unknown LAW", self.LAW)
        assert False

    def printid(self):
        return super().printid() + " AWP=%g, LIP=%i, LAW=%i"%(self.AWP, self.LIP, self.LAW)

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
        footer = ENDF_HEAD_Record(next(iterlines))
        assert footer.rectp == "SEND"

    def __repr__(self):
        s = self.printid() +', %g AMU; %i products in frame %i'%(self.AWR, self.NK, self.LCT)
        for t in self.sections: s += '\n'+str(t)
        return s
