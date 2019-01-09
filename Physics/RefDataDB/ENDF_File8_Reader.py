from ENDF_Base_Reader import *

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

class ENDF_File8_MT457_Spectrum(ENDF_List):
    """Spectrum data structure in File 8, section 457 data"""
    def __init__(self, iterlines):
        super().__init__(iterlines)
        self.rnm("C2","STYP")   # decay radiation type
        self.rnm("L1","LCON")   # continuum spectrum flag. 0: no continuous given; 1: only continuous given; 2: discrete and continuous given
        self.rnm("N2","NER")    # number of tabulated discrete energies

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
            self.info1 = ENDF_List(iterlines)
            self.T_h = self.info1.C1    # halflife
            self.dT_h = self.info1.C2   # uncertainty on T_h
            self.info2 = ENDF_List(iterlines)
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
        if self.MT == 457: s += "; T_1/2 = %g ~ %g"%(self.T_h, self.dT_h)
        return s

    def __repr__(self):
        s = self.printid()
        for t in self.products: s += '\n\t'+str(t)
        return s
