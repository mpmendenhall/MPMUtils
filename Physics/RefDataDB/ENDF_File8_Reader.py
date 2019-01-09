from ENDF_Base_Reader import *

# names for RTYP = 0 to 10
RTYP_names = ["gamma", "beta", "e.c./e+", "IT", "alpha", "n", "SF", "p", "e-", "xray", "?"]

def make_RTYP(r):
    ir = int(r)
    return (ir,) if r == ir else (ir, int(10*r-10*ir))
def name_RTYP(r): return ','.join([RTYP_names[i] for i in r])

class ENDF_File8_FPY(ENDF_HEAD_Record):
    """Fission Product Yields in File 8"""
    def __init__(self, iterlines):
        super().__init__(next(iterlines))
        assert self.MF == 8 and self.MT in (454,459)
        footer = ENDF_HEAD_Record(next(iterlines))
        assert footer.rectp == "SEND"

        self.rnm("L1","LE")
        self.LE -= 1
        self.contents = [ENDF_List(iterlines) for i in range(self.LE+1)]

        footer = ENDF_HEAD_Record(next(iterlines))
        assert footer.rectp == "SEND"

class ENDF_File8_MT457_DiscreteDecay(ENDF_List):
    def __init__(self,iterlines):
        super().__init__(iterlines)
        self.rnm("C1","ER")     # energy (eV)
        self.rnm("C2","dER")    # uncertainty

        # decay mode(s) of LIS state
        self.RTYP = make_RTYP(self.data[0])
        self.TYPE = self.data[1]        # beta/EC type
        self.RI   = self.data[2]        # (relative) intensity
        self.dRI  = self.data[3]        # uncertainty
        self.RIS  = self.data[4]        # STYP 0: internal pair formation, 2: positron intensity
        self.dRIS = self.data[5]        # uncertainty
        if len(self.data) >= 12:        # total, K, and L conversion electron coefficients
            self.RICC  = self.data[6]
            self.dRICC = self.data[7]
            self.RICK  = self.data[8]
            self.dRICK = self.data[9]
            self.RICL  = self.data[10]
            self.dRICL = self.data[11]
        self.data = ()

    def __repr__(self):
        s = "Discrete from %s decay mode"%name_RTYP(self.RTYP)
        s += ", E = %g ~ %g MeV, I = %g ~ %g"%(self.ER*1e-6, self.dER*1e-6, self.RI, self.dRI)
        return s

class ENDF_File8_MT457_Spectrum(ENDF_List):
    """Spectrum data structure in File 8, section 457 data"""
    def __init__(self, iterlines):
        super().__init__(iterlines)
        assert self.C1 == 0
        self.rnm("C2","STYP")   # decay radiation type
        self.STYP = make_RTYP(self.STYP)
        self.rnm("L1","LCON")   # continuum spectrum flag. 0: no continuous given; 1: only continuous given; 2: discrete and continuous given
        self.rnm("N2","NER")    # number of tabulated discrete energies
        assert self.L2 == 0
        assert len(self.data) == 6
        self.FD = self.data[0]  # discrete spectrum normalization, absolute / relative
        self.dFD = self.data[1] # uncertainty
        self.ERav = self.data[2]# average decay energy
        self.dERav = self.data[3] # uncertainty
        self.FC = self.data[4]  # continuum spectrum normalization, absolute / relative
        self.dFC = self.data[5] # uncertainty
        self.data = ()

        self.discrete = [ENDF_File8_MT457_DiscreteDecay(iterlines) for i in range(self.NER)] if self.LCON != 1 else None
        self.continuous = ENDF_Tab1(iterlines) if self.LCON != 0 else None # normalized spectrum RP(E) in prob/eV
        self.cov = ENDF_List(iterlines) if self.LCON and self.continuous.L2 else None

    def __repr__(self):
        s = name_RTYP(self.STYP) + " decay radiation, E_avg = %g ~ %g MeV"%(self.ERav * 1e-6, self.dERav * 1e-6)
        if self.discrete:
            if self.FD != 1 or self.dFD != 0: s += "\nDiscrete spectra (norm %g ~ %g)"%(self.FD, self.dFD)
            for t in self.discrete: s += "\n"+indent(str(t), '\t')
        if self.continuous:
            if self.FC != 1 or self.dFC != 0: s += "\nContinuous spectra (norm %g ~ %g)"%(self.FC, self.dFC)
            s += "\n"+indent(str(self.continuous), '\t')
        return s

class ENDF_File8_DecayProduct:
    def __init__(self,l):
        self.HL = l[0]      # half-life (seconds)
        self.RTYP = l[1]    # decay mode, as specified in MF8 MT457
        ZAN = int(l[2])     # 1000*Z + A
        self.Z = ZAN//1000
        self.A = ZAN%1000
        self.BR = l[3]      # branching fraction
        self.END = l[4]     # endpoint energy (eV)
        self.CT = l[5]      # chain terminator flags

    def __repr__(self):
        return "%i/%i\t%12g s\t%12g\t%12g eV\t[%f]"%(self.Z, self.A, self.HL, self.BR, self.END, self.CT)

class ENDF_File8_DecayProducts(ENDF_List):
    def __init__(self, iterlines):
        super().__init__(iterlines)

        self.rnm("C1","ZAP")    # nuclide produced in reaction
        self.rnm("C2","ELFS")   # excitation state of ZAP, eV
        self.rnm("L1","LMF")    # file containing cross section / multiplicity data
        self.rnm("L2","LMS")    # excitation state number of ZAP
        self.ND = self.NPL // 6 # number of decay product entries
        self.rectp = "%i Decay Products"%self.ND
        self.data = [ENDF_File8_DecayProduct(self.data[6*i : 6*i+6]) for i in range(self.ND)]
        self.ZAP = int(self.ZAP)
        self.Z = self.ZAP//1000
        self.A = self.ZAP%1000

    def __repr__(self):
        s = super().__repr__() + " product %i/%i (%i: %g eV) MF %i"%(self.Z, self.A, self.LMS, self.ELFS, self.LMF)
        for d in self.data: s += '\n\t'+str(d)
        return s

class ENDF_File8_FissProd:
    def __init__(self, l):
        ZAFP = int(l[0])    # 1000*Z + A
        self.Z = ZAFP//1000
        self.A = ZAFP%1000
        self.FPS = l[1]     # state designator
        self.Y = l[2]       # yield
        self.DY = l[3]      # uncertainty on Y

    def __repr__(self):
        return "%i/%i (%f)\t%12g ~ %12g"%(self.Z, self.A, self.FPS, self.Y, self.DY)


class ENDF_File8_FPY(ENDF_List):
    """File 8 fission product yields datastructure"""
    def __init__(self, iterlines):
        super().__init__(iterlines)

        self.rnm("C1","E")  # incident energy
        self.rnm("L1","I")  # interpolation scheme to previous energy point, if energy dependence given
        self.rnm("N2","NFP")# number of fission products specified
        self.rectp = "Fission Product Yields"

        self.data = [ENDF_File8_FissProd(self.data[4*i : 4*i+4]) for i in range(self.NFP)]

    def __repr__(self):
        s = super().__repr__() + ", incident E = %g eV (I=%i):"%(self.E, self.I)
        for d in self.data: s += '\n\t'+str(d)
        return s

class ENDF_File8_DecayEnergies(ENDF_List):
    """File 8 listing of decay energies"""
    def __init__(self, iterlines):
        super().__init__(iterlines)
        self.rectp = "Decay Energies"
        self.data = tuple(zip(*[iter(self.data)]*2))

    def __repr__(self):
        s = " * Average particles energy:"
        enames = ["light", "EM", "heavy", "e-", "e+", "Ae-", "Ce-", "gamma", "x-ray", "InB", "ann", "alpha", "recoil", "SF", "n", "p", "nu"]
        for i,d in enumerate(self.data): s += '\n\t%s\t%12g ~ %g MeV'%(enames[i], d[0]*1e-6, d[1]*1e-6)
        return s


class File8_DecayBranch:
    """One branch in decay listing"""
    def __init__(self, l):
        self.RTYP = make_RTYP(l[0])
        self.RFS = int(l[1])# daughter isomeric state number
        self.Q = l[2]       # transition Q-value
        self.dQ = l[3]      # uncertainty
        self.BR = l[4]      # branching fraction
        self.dBR = l[5]     # uncertainty

    def __repr__(self):
        return name_RTYP(self.RTYP) + " decay mode to daughter isomeric state %i\tQ = %g ~ %g MeV, BR = %g ~ %g"%(self.RFS, self.Q*1e-6, self.dQ*1e-6, self.BR, self.dBR)

class ENDF_File8_DecayBranches(ENDF_List):
    """File 8 listing of decay energies"""
    def __init__(self, iterlines):
        super().__init__(iterlines)
        self.rectp = "Decay Branches"
        self.data = [File8_DecayBranch(self.data[6*i:6*i+6]) for i in range(len(self.data)//6)]

    def __repr__(self):
        s = "" #" * Decay Branches:"
        for d in self.data: s += '\n'+str(d)
        return s


class ENDF_File8_Sec(ENDF_HEAD_Record):
    """File 8 'Decay and Fission Products Yields' section"""
    def __init__(self, iterlines, l0 = None):
        if l0: super().__init__(l0)
        else: super().__init__(next(iterlines))

        self.rectp = "File 8/%i "%(self.MT)
        if self.MT == 454: self.rectp += "Fission Products Independent Yields"
        elif self.MT == 457: self.rectp += "Radioactive Decay"
        elif self.MT == 459: self.rectp += "Fission Products Cumulative Yields,"
        else: self.rectp += "Radioactive Nuclide Production from process MT=%i"%self.MT

        self.rnm("L1","LIS")    # state number of target
        self.rnm("L2","LISO")   # isomeric state number of target

        self.products = []

        if self.MT in (454,459):
            self.LE = self.LIS - 1   # whether energy-dependent fission product yields given
            self.products = [ENDF_File8_FPY(iterlines) for i in range(self.LE+1)]

        elif self.MT == 457:
            self.rnm("N1","NST")    # nucleus stability: 0 = radioactive, 1 = stable
            self.rnm("N2","NSP")    # number of spectrum blocks
            if self.NST: assert not self.NSP
            self.decayE = ENDF_File8_DecayEnergies(iterlines)   # average total decay energies
            self.T_h = self.decayE.C1   # halflife
            self.dT_h = self.decayE.C2  # uncertainty on T_h
            self.branches = ENDF_File8_DecayBranches(iterlines)
            self.products = [ENDF_File8_MT457_Spectrum(iterlines) for i in range(self.NSP)]

        else:
            self.rnm("N1","NS") # number of states for which data is provided
            self.rnm("N2","NO") # 0: complete chain; 1: see MT=457 data

            if self.NO == 1: self.products = [ENDF_CONT_Record(next(iterlines)) for i in range(self.NS)]
            else: self.products = [ENDF_File8_DecayProducts(iterlines) for i in range(self.NS)]

        footer = ENDF_HEAD_Record(next(iterlines))
        assert footer.rectp == "SEND"

    def printid(self):
        s = super().printid() + " target state %i (iso. %i)"%(self.LIS, self.LISO)
        #if self.MT in (454,459): s += "; NS=%i%s"%(self.NS, "" if not self.NO else " +aux in MT457")
        if self.MT == 457:
            if self.NST: s += " STABLE"
            else: s += "; T_1/2 = %g ~ %g"%(self.T_h, self.dT_h)
        return s

    def __repr__(self):
        s = self.printid()
        if self.MT == 457 and not self.NST:
            #s += "\n"+str(self.decayE)
            s += "\n"+indent(str(self.branches),'\t')
        #if self.products: s += "\n * Products:"
        #for t in self.products: s += '\n'+indent(str(t), '\t')
        return s
