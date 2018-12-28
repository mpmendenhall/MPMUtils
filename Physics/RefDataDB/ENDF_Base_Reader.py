
# package for parsing Fortran-formatted text files
# pip install --user fortranformat
import fortranformat as ff

# for 'super()' in python2
from builtins import *

class ENDF_Record(object):
    """Generic record (line in file) parser"""
    record_reader = ff.FortranRecordReader('(A66,I4,I2,I3,I5)')

    def __init__(self, line, rt="TEXT"):
        self.TEXT, self.MAT, self.MF, self.MT, self.NS = self.record_reader.read(line)
        self.rectp = rt
        # MAT: material identifier
        # MF: file number (type of information)
        # MT: section number in file
        # NS: line number
        # TEXT: uninterpreted text contents

    def printid(self):
        """Short-form description"""
        return "[m%i f%i s%i l%i] "%(self.MAT, self.MF, self.MT, self.NS) + self.rectp

    def __repr__(self):
        return self.printid() + ' "%s"'%self.TEXT


class ENDF_CONT_Record(ENDF_Record):
    """CONTROL record, generic base type for many entries"""
    cont_reader = ff.FortranRecordReader('(2E11.0,4I11)')

    def __init__(self, line):
        super().__init__(line, "CONT")
        self.C1, self.C2, self.L1, self.L2, self.N1, self.N2 = self.cont_reader.read(self.TEXT)
        # C1, C2         floating-point values
        # L1, L2, N1, N2 integer control values
        if self.N1 == self.N2 == self.C1 == self.C2 == self.L1 == self.L2 == 0:
            self.rectp = "SEND"
            if self.MF == 0: self.rectp = "FEND"    # file end
            if self.MAT == 0: self.rectp = "MEND"   # material end
            if self.MAT == -1: self.rectp = "TEND"  # tape end

    def parse(line, withUID = False):
        l = ENDF_Record(line)
        ldat = ENDF_CONT_Record.cont_reader.read(l.text)
        if withUID: return ldat + l.uid()
        return ldat

    def __repr__(self):
        return self.printid() +' C1,C2=%g,%g L1,L2=%i,%i N1,N2=%i,%i'%(self.C1, self.C2, self.L1, self.L2, self.N1, self.N2)


class ENDF_HEAD_Record(ENDF_CONT_Record):
    """HEAD = CONT, with C1,C2 -> ZA,AWR"""
    def __init__(self,line):
        super().__init__(line)
        self.ZA = self.C1  # 1000*Z + A
        self.AWR = self.C2 # Mass (AMU)
        if self.rectp == "CONT": self.rectp = "HEAD"

    def printid(self):
        return super().printid() +' ZA %i/%i'%(self.ZA//1000, self.ZA%1000)

    def __repr__(self):
        return self.printid() +'; L1,L2=%i,%i N1,N2=%i,%i'%(self.L1, self.L2, self.N1, self.N2)


class ENDF_List(ENDF_HEAD_Record):
    """List data format"""
    list_reader = ff.FortranRecordReader('(6E11.0)')

    def __init__(self,iterlines):
        super().__init__( next(iterlines))
        self.NPL = self.N1 # number of entries
        self.rectp = "LIST(%i)"%self.NPL

        dat = [self.list_reader.read(ENDF_Record(next(iterlines)).TEXT) for i in range((self.NPL+5)//6)]
        self.data = [j for i in dat for j in i][:self.NPL]


class ENDF_Tab1(ENDF_HEAD_Record):
    """(x,y) table data format [MAT,MF,MT/ C1, C2, L1, L2, NR, NP/x int /y(x)]TAB1"""
    range_reader = ff.FortranRecordReader('(6I11)')
    pair_reader = ff.FortranRecordReader('(6E11.0)')

    def __init__(self, iterlines):
        super().__init__(next(iterlines))
        self.NR = self.N1    # number of ranges
        self.NP = self.N2    # number of x,y pairs (split into interpolation ranges)
        self.rectp = "TAB1(%ix%i)"%(self.NR, self.NP)
        self.ux, self.uy = None,None

        rdat = [self.range_reader.read(ENDF_Record(next(iterlines)).TEXT) for i in range((self.NR+2)//3)]
        rdat = [j for i in rdat for j in i][:2*self.NR]
        self.ranges = list(zip(*[iter(rdat)]*2))

        pdat = [self.pair_reader.read(ENDF_Record(next(iterlines)).TEXT) for i in range((self.NP+2)//3)]
        pdat = [j for i in pdat for j in i][:2*self.NP]
        self.xy = list(zip(*[iter(pdat)]*2))

    def __repr__(self):
        s = '\n' + self.printid() + '\n'
        i0 = 0
        for r in self.ranges:
            s += "-- range interpolation %i --\n"%r[1]
            for i in range(r[0]): s += "\t%g\t%g\n" % self.xy[i+i0]
            i0 += r[0]
        return s + "------- end TAB1 -------\n"

class ENDF_Tab2(ENDF_HEAD_Record):
    """2-dimensional table data format [MAT,MF,MT/ C1, C2, L1, L2, NR, NZ/ Z int ]TAB2"""
    range_reader = ff.FortranRecordReader('(6I11)')

    def __init__(self, iterlines):
        super().__init__(next(iterlines))
        self.NR = self.N1    # number of ranges
        self.NZ = self.N2    # number of 1D sub-tables
        self.rectp = "TAB2(%ix%i)"%(self.NR, self.NZ)

        rdat = [self.range_reader.read(ENDF_Record(next(iterlines)).TEXT) for i in range((self.NR+2)//3)]
        rdat = [j for i in rdat for j in i][:2*self.NR]
        self.ranges = list(zip(*[iter(rdat)]*2))

        self.entries = [] # loaded afterwards for each NZ, depending on file structure

    def __repr__(self):
        s = '\n' + self.printid() + '\n'
        for e in self.entries: s += str(e)+'\n'
        return s + "------- end TAB2 -------\n"



