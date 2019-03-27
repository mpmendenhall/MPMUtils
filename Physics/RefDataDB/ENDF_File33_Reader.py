from ENDF_Base_Reader import *

class ENDF_File33_NC_SSSec(ENDF_List):
    """File 33 NC-type subsubsection"""
    def __init__(self, iterlines):
        h = ENDF_CONT_Record(next(iterlines))
        super().__init__(iterlines)
        self.LTY = h.L2 # Flag for procedure to obtain covariance matrix
        assert self.LTY in (0,1,2,3,4)
        self.rectp += " NC"

class ENDF_File33_NI_SSSec(ENDF_List):
    """File 33 NI-type subsubsection"""
    def __init__(self, iterlines):
        super().__init__(iterlines)
        self.rectp += " NI"
        self.rnm("L2","LB") # flag for interpretation of contents


        if self.LB in (0,1,2,3,4):
            self.rnm("N2","NP") # total number of E,F pairs
            self.rnm("L1","LT") # number of pairs in second array
            n = 2*(self.NP-self.LT)
            self.Ek = self.data[0:n][::2]
            self.Fk = self.data[1:n][::2]
            self.El = self.data[n:][::2]
            self.Fl = self.data[n+1:][::2]
            self.data = []
            self.rectp += " {%i %g -- %g"%(self.NP-self.LT, self.Ek[0], self.Ek[-1])
            if self.LT: self.rectp += ", %i %g -- %g"%(self.LT, self.El[0], self.El[-1])
            self.rectp += "}"

        elif self.LB == 5:      # space-efficient energy-averaged relative covariance matrix representation
            self.rnm("N2","NE") # number of energy bin edge entries
            self.rnm("L1","LS") # symmetric matrix flag
            self.Er = self.data[:self.NE]
            self.rectp += " {%i^2 %g -- %g%s}"%(self.NE-1, self.Er[0], self.Er[-1], " S" if self.LS else "")

            if not self.LS: # asymetric
                self.data = [self.data[self.NE + i*(self.NE-1) : self.NE + (i+1)*(self.NE-1)] for i in range(self.NE-1)]
            else: # symmtric data packing
                d = []
                n = self.NE
                for i in range(self.NE-1):
                    n2 = n + self.NE - 1 - i
                    d.append(self.data[n : n2])
                    n = n2
                self.data = d

        elif self.LB == 6:    # Non-square matrix for different reactions/materials
            self.rnm("N2","NER")                # Number of energies defining rows (NER-1 intervals)
            self.NEC = (self.NPL - 1)//self.NER # number of column energies
            self.Er = self.data[0:self.NER]     # Row energy bin edges
            self.Ec = self.data[self.NER:self.NER+self.NEC] # Column energy bin edges
            self.data = self.data[self.NER+self.NEC:]       # (NER-1)*(NEC-1) array
            self.data = [ self.data[i*(self.NEC-1) : (i+1)*(self.NEC-1)] for i in range(self.NER-1)]
            self.rectp += " {%i %g -- %g x %i %g -- %g}"%(self.NER-1, self.Er[0], self.Er[-1], self.NEC-1, self.Ec[0], self.Ec[-1])

        else: # LB = 8,9 not yet handled
            pass

    def printid(self):
        return super().printid() + " LB=%i"%self.LB

    def __repr__(self):
        s = self.printid()
        if self.LB < 5 and len(self.Ek) < 20:
            for i in range(len(self.Ek)):
                s += "\n\t%12g\t%g"%(self.Ek[i],self.Fk[i])
        return s

class ENDF_File33_SubSec(ENDF_CONT_Record):
    """File 33 subsection [MAT,33,MT/ XMF1, XLFS1, MAT1, MT1, NC, NI]CONT"""
    def __init__(self, iterlines):
        super().__init__(next(iterlines))
        self.XMF1  = int(self.C1)   # MF for the 2nd cross section of the pair; if MF1=MF, XMF1=0.0 or blank.
        self.XLFS1 = int(self.C2)   # final excited state of the 2nd energy-dependent cross section.
        self.rnm("L1","MAT1")       # MAT for 2nd cross-section
        self.rnm("L2","MT1")        # MT for 2nd cross-section
        self.rnm("N1","NC")         # number of NC-type subsubsections
        self.rnm("N2","NI")         # number of NI-type subsubsections
        self.rectp = "Covariance x [m%i(%i) f%i s%i]"%(self.MAT1, self.XLFS1, self.XMF1, self.MT1)

        self.subNC = [ENDF_File33_NC_SSSec(iterlines) for i in range(self.NC)]
        self.subNI = [ENDF_File33_NI_SSSec(iterlines) for i in range(self.NI)]

    def __repr__(self):
        s = self.printid()
        if self.subNC:
            s += "\n---- NC subsubsections ----"
            for ss in self.subNC: s += '\n' + str(ss)
        if self.subNI:
            s += "\n---- NI subsubsections ----"
            for ss in self.subNI: s += '\n' + str(ss)
        return s


class ENDF_File33_Sec(ENDF_HEAD_Record):
    """File 33 'Covariances of neutron cross sections'"""

    def __init__(self, iterlines, l0 = None):
        if l0 is None: l0 = next(iterlines)
        super().__init__(l0)
        assert self.MF == 33
        self.rectp = "File 33 'Covariances of neutron cross sections' section %i"%self.MT

        self.MTL = self.L2  # Nonzero: flags one component of lumped reaction; cov. not given
        self.NL  = self.N2  # number of subsections (0 in lumped MTL != 0 files)
        assert not (self.MTL and self.NL)

        self.contents = [ENDF_File33_SubSec(iterlines) for i in range(self.NL)]

        footer = ENDF_HEAD_Record(next(iterlines))
        assert footer.rectp == "SEND"

    def __repr__(self):
        s = self.printid()
        for c in self.contents: s += '\n\n'+str(c)
        s += "\n\n----- End File 33 ------"
        return s
