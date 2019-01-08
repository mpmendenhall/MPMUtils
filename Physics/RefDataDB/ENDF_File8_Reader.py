from ENDF_Base_Reader import *

class ENDF_File8_FPY(ENDF_HEAD_Record):
    """Fission Product Yields in File 8"""
    def __init__(self, iterlines):
        super().__init__(next(iterlines))
        assert self.MF == 8 and self.MT in (454,459)
        footer = ENDF_HEAD_Record(next(iterlines))
        assert footer.rectp == "SEND"

        self.LE = self.L1 - 1
        self.contents = [ENDF_List(iterlines) for i in range(self.L1)]

        footer = ENDF_HEAD_Record(next(iterlines))
        assert footer.rectp == "SEND"

class ENDF_File8_MT457_Spectrum(ENDF_List):
    """Spectrum data structure in File 8, section 457 data"""
    def __init__(self, iterlines):
        super().__init__(iterlines)
        self.STYP = self.C2 # decay radiation type
        self.LCON = self.L1 # continuum spectrum flag. 0: no continuous given; 1: only continuous given; 2: discrete and continuous given
        self.NER = self.N2  # number of tabulated discrete energies

        self.discrete = [ENDF_List(iterlines) for i in range(self.NER)] if self.LCON != 1 else None
        self.continuous = ENDF_Tab1(iterlines) if self.LCON != 0 else None
        self.cov = ENDF_List(iterlines) if self.LCON and self.continuous.L2 else None

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

        ZAP = int(self.C1)      # nuclide produced in reaction
        self.Z = ZAP//1000
        self.A = ZAP%1000
        self.ELFS = self.C2     # excitation state of ZAP, eV
        self.LMF = self.L1      # file containing cross section / multiplicity data
        self.LMS = self.L2      # excitation state number of ZAP
        self.ND = self.N1 // 6  # number of decay product entries
        self.rectp = "%i Decay Products"%self.ND
        self.data = [ENDF_File8_DecayProduct(self.data[6*i : 6*i+6]) for i in range(self.ND)]

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

        self.E = self.C1    # incident energy
        self.I = self.L1    # interpolation scheme to previous energy point, if energy dependence given
        self.NFP = self.N2  # number of fission products specified
        self.rectp = "Fission Product Yields"

        self.data = [ENDF_File8_FissProd(self.data[4*i : 4*i+4]) for i in range(self.NFP)]

    def __repr__(self):
        s = super().__repr__() + ", incident E = %g eV (I=%i):"%(self.E, self.I)
        for d in self.data: s += '\n\t'+str(d)
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

        self.LIS  = self.L1 # state number of target
        self.LISO = self.L2 # isomeric state number of target

        self.products = []

        if self.MT in (454,459):
            self.LE = self.L1 - 1   # whether energy-dependent fission product yields given
            self.products = [ENDF_File8_FPY(iterlines) for i in range(self.LE+1)]

        elif self.MT == 457:
            self.NST = self.N1 # nucleus stability: 0 = radioactive, 1 = stable
            self.NSP = self.N2 # number of spectrum blocks
            if self.NST: assert not self.NSP
            self.info1 = ENDF_List(iterlines)
            self.T_h = self.info1.C1    # halflife
            self.dT_h = self.info1.C2   # uncertainty on T_h
            self.info2 = ENDF_List(iterlines)
            self.products = [ENDF_File8_MT457_Spectrum(iterlines) for i in range(self.NSP)]

        else:
            self.NS   = self.N1 # number of states for which data is provided
            self.NO   = self.N2 # 0: complete chain; 1: see MT=457 data

            if self.NO == 1: self.products = [ENDF_CONT_Record(next(iterlines)) for i in range(self.NS)]
            else: self.products = [ENDF_File8_DecayProducts(iterlines) for i in range(self.NS)]

        footer = ENDF_HEAD_Record(next(iterlines))
        assert footer.rectp == "SEND"

    def printid(self):
        s = super().printid() + " target state %i (iso. %i)"%(self.LIS, self.LISO)
        #if self.MT in (454,459): s += "; NS=%i%s"%(self.NS, "" if not self.NO else " +aux in MT457")
        if self.MT == 457: s += "; T_1/2 = %g ~ %g"%(self.T_h, self.dT_h)
        return s

    def __repr__(self):
        s = self.printid()
        for t in self.products: s += '\n\t'+str(t)
        return s
