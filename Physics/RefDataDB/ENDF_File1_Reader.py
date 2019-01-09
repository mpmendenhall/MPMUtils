from ENDF_Base_Reader import *

class ENDF_File1_Sec451(ENDF_HEAD_Record):
    """File 1 'General Information' section"""
    def __init__(self, iterlines, l0 = None):
        if l0 is None: l0 = next(iterlines)
        super().__init__(l0)
        assert self.MF == 1
        self.rectp = "File 1 section %i"%self.MT

        if self.MT == 451:
            self.rectp += " 'General Information'"

            self.rnm("L1","LRP")    # resonance parameters flag
            self.rnm("L2","LFI")    # fission flag (1: fissions)
            self.rnm("N1","NLIB")   # library identifier
            self.rnm("N2","NMOD")   # modification number

            c = ENDF_CONT_Record(next(iterlines))
            self.ELIS = c.C1        # excitation energy of target relative to ground state
            self.STA  = c.C2        # target stability (1: unstable)
            self.LIS  = c.L1        # target state number
            self.LISO = c.L2        # isomeric state number
            assert self.LIS >= self.LISO
            self.NFOR = c.N2        # library format (6 for ENDF6)
            assert self.NFOR == 6

            c = ENDF_CONT_Record(next(iterlines))
            self.AWI = c.C1         # projectile mass, neutron units
            self.EMAX = c.C2        # highest energy for evaluation
            self.LREL = c.L1        # library release number, = 2 for ENDF/B-VI.2
            self.NSUB = c.N1        # sub-library number
            self.NVER = c.N2        # library version number, = 7 for ENDF/B-VII

            c = ENDF_CONT_Record(next(iterlines))
            self.TEMP = c.C1        # target temperature for derived data with doppler broadening
            self.LDRV = c.L1        # derived material flag
            self.NWD = c.N1         # number of descriptive text records
            self.NXC = c.N2         # number of directory records

            c = ENDF_Record(next(iterlines)).TEXT
            self.ZSYMAM = c[0:11].strip()
            self.ALAB   = c[11:22].strip()
            self.EDATE  = c[22+5:32].strip(" -")
            self.AUTH   = c[33:66].strip()

            c = ENDF_Record(next(iterlines)).TEXT
            self.REF    = c[1:22].strip()
            self.DDATE  = c[22+5:32].strip(" -")
            self.RDATE  = c[33:43]
            self.ENDATE = c[52:63].strip()

            self.txt = [ENDF_Record(next(iterlines)).TEXT for i in range(self.NWD-2)]

            self.directory = []
            for i in range(self.NXC):
                c = ENDF_CONT_Record(next(iterlines))
                self.directory.append((c.L1, c.L2, c.N1, c.N2))

        elif self.MT in (452, 456):
            if self.MT == 452:  self.rectp += " 'Number of neutrons per fission'"
            else: self.rectp += " 'Number of prompt neutrons per fission'"
            self.rnm("L2","LNU")    # format flag, 1: polynomial, 2: tabulated
            assert self.LNU in (1,2)
            if self.LNU == 1: self.nubar = ENDF_List(iterlines)
            elif self.LNU == 2: self.nubar = ENDF_Tab1(iterlines)

        elif self.MT == 455:
            self.rectp = "File 1 section %i 'Delayed neutron data'"%self.MT
            self.rnm("L1","LDG")    # 0: energy-independent, 1: energy-dependent
            self.rnm("L2","LNU")    # format flag, 1: polynomial, 2: tabulated
            assert self.LNU in (1,2)
            raise NotImplementedError

        else: raise NotImplementedError

        footer = ENDF_HEAD_Record(next(iterlines))
        assert footer.rectp == "SEND"

    def __repr__(self):
        if self.MT == 451:
            s = "## File 1 Section 451 'General Information'; "
            s += "Library %i.%i%s, format %i. Modification %i."%(self.NVER, self.LREL, " [%i]"%self.NLIB if self.NLIB else "", self.NFOR, self.NMOD)
            s += "\n## %s Evaluated %s by %s (%s): %s"%(self.ZSYMAM, self.EDATE, self.AUTH, self.ALAB, self.REF)
            s += "\n## Revised %s; first distributed %s; entered %s."%(self.RDATE, self.DDATE, self.ENDATE)
            s += "\n## Target material %i: Z/A %i/%i, mass %g m_n; in state %i(%i) excited %g eV."%(self.MAT, self.Z, self.A, self.AWR, self.LIS, self.LISO, self.ELIS)
            if self.LDRV: s += "\n## Derived material number %g."%self.LDRV
            if self.STA: s += "\n##\ttarget is unstable."
            if self.LFI: s += "\n##\ttarget is fissionable."
            if self.TEMP: s += "\n##\ttarget temperature %g K."%self.TMP
            s += "\n## Projectile  mass %g m_n, up to %g MeV"%(self.AWI, self.EMAX/1e6)

            s += "\n##\n## Included files:"
            for d in self.directory: s += "\n## MF %3i\tMT %3i\t%6i records\tModification %i"%d

            s += "\n##\n"+"#"*72
            for t in self.txt: s += '\n## ' + t + " ##"
            s += "\n"+"#"*72
            return s
        else: return super().__repr__()

def ENDF_File1_Sec(iterlines, l0 = None):
    if l0 is None: l0 = next(iterlines)
    h = ENDF_HEAD_Record(l0)
    assert h.MF == 1

    f = None
    if h.MT == 451: f = ENDF_File1_Sec451(iterlines, l0)

    return f
