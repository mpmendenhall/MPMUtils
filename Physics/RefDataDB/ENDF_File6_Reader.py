from ENDF_Base_Reader import *

class ENDF_File6_LAW1_List(ENDF_List):
    """List entry in LAW=1 File 6 table"""
    def __init__(self, iterlines):
        super().__init__(iterlines)
        self.rnm("C2","E1") # incident energy
        self.rnm("N2","NEP")# number of secondary energies listed
        self.rnm("L1","ND") # number of discrete energies listed
        self.rnm("L2","NA") # number of angular parameters

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
        self.rnm("L1","LANG")
        # Interpolation scheme for secondary energy:
        # (1) histogram,
        # (2) linear-linear, etc.
        self.rnm("L2","LEP")

    def printid(self):
        return super().printid() + ' LANG=%i, LEP=%i'%(self.LANG, self.LEP)


class ENDF_File6_LAW2_List(ENDF_List):
    """List entry in LAW=2 File 6 table [MAT, 6, MT/ 0.0, E_1 ,LANG, 0, NW, NL/ A_l(E)]LIST"""
    def __init__(self, iterlines):
        super().__init__(iterlines)
        self.rnm("C2","E1")     # incident energy [eV]
        self.rnm("L1","LANG")   # angular distribution type
        self.rnm("N2","NL")     # LANG=0: highest Legendre order used; else number of cosines tabulated.
        self.rectp = "LIST.LAW2"

    def __repr__(self):
        s = self.printid() +'; E1 %g MeV,\tLANG = %i'%(1e-6 * self.E1, self.LANG)
        if self.LANG == 0:
            return s + ': Legendre polynomial with NL = %i terms:  \t'%self.NL + str(self.data)
        return s+',\tNL=%i'%self.NL

class ENDF_File6_LAW2(ENDF_Tab2):
    """LAW=2 angular distribution"""
    def __init__(self, iterlines):
        super().__init__(iterlines, ENDF_File6_LAW2_List)
        self.rectp += " Discrete 2-body scattering"

class ENDF_File6_LAW5(ENDF_Tab2):
    """LAW=5 angular distribution"""
    def __init__(self, iterlines):
        super().__init__(iterlines, ENDF_List)
        self.rectp += " Charged particle elastic scattering"

class ENDF_File6_LAW6(ENDF_CONT_Record):
    """LAW=6 n-body phase space"""
    def __init__(self, iterlines):
        super().__init__(next(iterlines))
        self.rectp = "N-body phase space"
        self.rnm("C1","APSX")   # total mass (neutron units) of particles
        self.rnm("N2","NPSX")   # number of particles

    def __repr__(self):
        return self.rectp + " N=%i, A=%g"%(self.NPSX, self.APSX)

class ENDF_File26_LAW8(ENDF_Tab1):
    """LAW=8 angular distribution for bremsstrahlung """
    def __init__(self, iterlines):
        super().__init__(iterlines)
        self.rectp += " Excitation energy transfer"

class ENDF_File6_Tab1(ENDF_Tab1):
    """Table in File 6 data"""
    def __init__(self, iterlines):
        super().__init__(iterlines)
        self.rectp += " Products Distribution"
        self.rnm("C1","ZAP")    # product 1000*Z + A
        self.rnm("C2","AWP")    # product mass, neutron units; or, energy of primary photon for ZAP=0
        self.rnm("L1","LIP")    # product modifier flag
        self.rnm("L2","LAW")    # distribution type
        self.ZAP = int(self.ZAP)
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
        if self.LAW == 5: self.distrib = ENDF_File6_LAW5(iterlines)
        if self.LAW == 6: self.distrib = ENDF_File6_LAW6(iterlines)
        if self.LAW == 8: self.distrib = ENDF_File26_LAW8(iterlines)
        if self.distrib: return

        print("Unknown LAW", self.LAW)
        raise NotImplementedError

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
        assert self.MF%20 == 6
        self.rectp = "File %i 'Product Energy-Angle Distributions' section %i"%(self.MF, self.MT)
        self.rnm("N2","LCT")    # reference frame for secondary energy, angle specification
        self.rnm("N1","NK")     # number of subsections

        self.sections = [ENDF_File6_Tab1(iterlines) for i in range(self.NK)]
        footer = ENDF_HEAD_Record(next(iterlines))
        assert footer.rectp == "SEND"

    def __repr__(self):
        s = self.printid() +', %g AMU; %i products in frame %i'%(self.AWR, self.NK, self.LCT)
        for t in self.sections: s += '\n    --- File 6 Section ---' + str(t)
        return s
