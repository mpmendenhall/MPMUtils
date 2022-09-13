
# package for parsing Fortran-formatted text files
# pip3 install --user fortranformat
import fortranformat as ff

# for 'super()' in python2
# from builtins import *

from bisect import bisect
from textwrap import indent
from math import *

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

        self.endlvl = 0
        if self.MT == 0: self.rectp = "SEND"; self.endlvl = 1   # section end
        if self.MF == 0: self.rectp = "FEND"; self.endlvl = 2   # file end
        if self.MAT == 0: self.rectp = "MEND"; self.endlvl = 3  # material end
        if self.MAT == -1: self.rectp = "TEND"; self.endlvl = 4 # tape end

    def printid(self):
        """Short-form description"""
        return "[MAT %i MF %i MT %i line %i] "%(self.MAT, self.MF, self.MT, self.NS) + self.rectp

    def __repr__(self):
        return self.printid() + ' "%s"'%self.TEXT

    def rnm(self, old_name, new_name):
        """Rename attribute"""
        self.__dict__[new_name] = self.__dict__.pop(old_name)


class ENDF_CONT_Record(ENDF_Record):
    """CONTROL record, generic base type for many entries [MAT,MF,MT/C1,C2,L1,L2,N1,N2]CONT"""
    cont_reader = ff.FortranRecordReader('(2E11.0,4I11)')

    def __init__(self, line):
        super().__init__(line, "CONT")
        # C1, C2         floating-point values
        # L1, L2, N1, N2 integer control values
        self.C1, self.C2, self.L1, self.L2, self.N1, self.N2 = self.cont_reader.read(self.TEXT)
        del self.TEXT

    def __repr__(self):
        return self.printid() +' C1,C2=%g,%g L1,L2=%i,%i N1,N2=%i,%i'%(self.C1, self.C2, self.L1, self.L2, self.N1, self.N2)


class ENDF_HEAD_Record(ENDF_CONT_Record):
    """HEAD = CONT, with C1,C2 -> ZA,AWR"""
    def __init__(self,line):
        super().__init__(line)
        self.rnm("C1", "ZA")    # 1000*Z + A
        self.rnm("C2", "AWR")   # Mass (AMU)
        self.ZA = int(self.ZA)
        self.Z = self.ZA//1000
        self.A = self.ZA%1000
        if self.rectp == "CONT": self.rectp = "HEAD"

    def printid(self):
        return super().printid() +' ZA %i/%i, AWR=%g'%(self.Z, self.A, self.AWR)

    def __repr__(self):
        return self.printid() +'; L1,L2=%i,%i N1,N2=%i,%i'%(self.L1, self.L2, self.N1, self.N2)


class ENDF_List(ENDF_CONT_Record):
    """List data format [MAT,MF,MT/ C1, C2, L1, L2, NPL, N2/ B(n)] LIST"""
    list_reader = ff.FortranRecordReader('(6E11.0)')
    display_max = 24

    def __init__(self,iterlines):
        super().__init__( next(iterlines))
        self.rnm("N1","NPL")    # number of entries
        self.rectp = "LIST(%i)"%self.NPL

        dat = [self.list_reader.read(ENDF_Record(next(iterlines)).TEXT) for i in range((self.NPL+5)//6)]
        self.data = [j for i in dat for j in i][:self.NPL]

    def printid(self):
        return " * " + self.rectp

    def __repr__(self):
        s = self.printid()
        if len(self.data) <= ENDF_List.display_max: s += '\t' + str(self.data)
        return  s


class ENDF_Ranges(ENDF_CONT_Record):
    """Base for tabulated range datastructures"""
    range_reader = ff.FortranRecordReader('(6I11)')
    INT_names = {1: "nearest", 2: "lin/lin", 3: "log/lin", 4: "lin/log", 5: "log/log"}

    def __init__(self, iterlines):
        super().__init__(next(iterlines))
        self.rnm("N1","NR") # number of ranges
        self.rnm("N2","NP") # number of points

        rdat = [self.range_reader.read(ENDF_Record(next(iterlines)).TEXT) for i in range((self.NR+2)//3)]
        rdat = [j for i in rdat for j in i][:2*self.NR]
        self.NBT = rdat[0::2]   # dividing point between ranges
        self.INT = rdat[1::2]   # range interpolation scheme

    def locate(self, x):
        """Return point lower-bound index and interpolation scheme"""
        b = bisect(self.xs, x)
        if b < 1 or b >= len(self.xs): return None, None
        return b, self.INT[bisect(self.NBT, b)]

    def printid(self):
        return super().printid() + " {%i/%i}"%(self.NP, self.NR)

    def interpl(x, x0, y0, x1, y1, i):
        """Interpolated between points"""
        if i == 1: return y0

        if i == 3 or i == 5: l = log(x/x0)/log(x1/x0)
        else:                l = (x-x0)/(x1-x0)

        if i == 2 or i == 3: return y0 + (y1 - y0)*l
        if i == 4 or i == 5: return exp(log(y0) + log(y1/y0)*l)

        # unsupported interpolation scheme
        raise NotImplementedError

class InterpolationPoints:
    """Interpolation between (energy) points"""
    def __init__(self, binedges = [], binterps = []):
        self.binedges = binedges    # bin edge positions
        self.binterps = binterps    # interpolation used within each bin

    def locate(self, x):
        """Locate bin number and fractional component 0 <= l <= 1"""
        if len(self.binedges) == 1: return 0,0
        b = bisect(self.binedges, x)
        if b < 1 or b >= len(self.binedges): return None, None
        i = self.binterps[b-1]

        if i == 1: return b-1, 0
        x0 = self.binedges[b-1]
        x1 = self.binedges[b]
        if i == 3 or i == 5: l = log(x/x0)/log(x1/x0)
        else:                l = (x-x0)/(x1-x0)
        return b-1, l




class ENDF_Tab1(ENDF_Ranges):
    """(x,y) table data format [MAT,MF,MT/ C1, C2, L1, L2, NR, NP/x int /y(x)]TAB1"""
    pair_reader = ff.FortranRecordReader('(6E11.0)')

    def __init__(self, iterlines):
        super().__init__(iterlines)
        self.ux, self.uy = None,None
        self.rectp = "TAB1"

        pdat = [self.pair_reader.read(ENDF_Record(next(iterlines)).TEXT) for i in range((self.NP+2)//3)]
        pdat = [j for i in pdat for j in i][:2*self.NP]
        self.xs = pdat[0::2]
        self.ys = pdat[1::2]

    def __call__(self, x, NoneIfOutside = False):
        """Evaluate at specified position"""
        b,i = self.locate(x)
        if b is None: return None if NoneIfOutside else 0.

        y0 = self.ys[b-1]
        if i == 1: return y0

        y1 = self.ys[b]
        return self.__class__.interpl(x, self.xs[b-1], y0, self.xs[b], y1, i)

    def __repr__(self):
        s = '\n' + self.printid() + '\n'
        i0 = 0
        for n,r in enumerate(self.NBT):
            s += "-- range interpolation %i (%s) --\n"%(self.INT[n], self.INT_names.get(self.INT[n], "???"))
            for i in range(i0, r): s += "\t%3i\t%12g\t%g\n" % (i, self.xs[i], self.ys[i])
            i0 = r
        return s + "------- end TAB1 -------\n"


class ENDF_Tab2(ENDF_Ranges):
    """2-dimensional table data format [MAT,MF,MT/ C1, C2, L1, L2, NR, NZ/ Z_int ]TAB2"""

    def __init__(self, iterlines, subelementClass = None):
        super().__init__(iterlines)
        self.entries = [subelementClass(iterlines) for i in range(self.NP)] if subelementClass else []
        self.rectp = "TAB2"

    def __repr__(self):
        s = '\n' + self.printid()
        i0 = 0
        for n,r in enumerate(self.NBT):
            s += "\n-- range interpolation %i (%s) --\n"%(self.INT[n], self.INT_names.get(self.INT[n], "???"))
            if self.entries:
                for i in range(i0, r): s += "\n%3i:"%i + indent(str(self.entries[i]), '\t')
            i0 = r
        return s + "\n------- end TAB2 -------\n"


#######################################
#######################################

def pop_section_lines(iterlines, endlvl=1):
    """Pop lines from iterator until control type found"""
    ls = []
    l = next(iterlines)
    while l:
        ls.append(l)
        r = ENDF_Record(l)
        if r.endlvl >= endlvl: break
        l = next(iterlines)
    return ls

