#!/bin/env python3

from textwrap import indent
from argparse import ArgumentParser

def optfloat(s):
    """Try to interpret string as float; None if blank; string if unparseable"""
    s = s.strip()
    if not s: return None
    try: return float(s)
    except: return s

def parse_stderr(s1,s2):
    """Interpret ENSDF number with standard error notation"""
    s1 = s1.strip()
    s2 = s2.strip()
    if not s1: return (None,None)

    try: v = float(s1)
    except: return (s1,None)

    if not s2: return (v,None)
    try: u = int(s2)
    except: return (v,s2)

    dvs = list(s1)
    i = s1.find("E")-1
    if i < 0: i = len(dvs)-1
    j = len(s2)-1
    while i >= 0:
        if dvs[i].isdigit():
            dvs[i] = s2[j] if j >= 0 else '0'
            j -= 1
        i -= 1
    e = float(''.join(dvs))
    return (v,e)

def parse_unit_stderr(s1,s2):
    """parse ENSDF number with units"""
    s1 = s1.strip()
    if s1[:1] == "(": return s1,s2
    s1s = s1.split(' ')
    v,e = parse_stderr(s1s[0],s2)

    unit = s1s[1].strip().upper() if len(s1s) == 2 else ""

    uvals = {"Y":365.25636*24*3600, "D": 24*3600, "H":3600,
                "M":60., "S":1., "MS":1e-3, "US":1e-6, "NS":1e-9,
                "PS":1e-12, "FS":1e-15, "AS":1e-18,
                "EV": 1e-3, "KEV":1., "MEV":1e3,
                "%":0.01, "":1}
    if unit not in uvals: return s1,s2
    umul = uvals[unit]

    if v: v *= umul
    if type(e) == type(1.0): e *= umul
    return (v,e)

class ENSDF_Record:
    """Generic ENSDF line"""
    def __init__(self, line):
        self.NUCID = line[:5]
        self.rectp = line[5:9]
        self.xdata = {} # {quantity: ([(op1, val1), (op2, val2)], XREF) ... }
        self.comments = []  # comments on this record

    def printid(self):
        """Short-form description"""
        return "[%s] '%s'"%(self.NUCID, self.rectp)

    def __repr__(self):
        """Extended description"""
        s = self.printid()
        for c in self.comments: s += '\n'+str(c)+'\n'
        for x in self.xdata:
            xd = self.xdata[x]
            s += "\n\t" + x + "\t" + '\t'.join([op+" "+str(v) for op,v in xd[0]])
            if xd[1]: s += "\t(%s)"%xd[1]
        return s


    def addCont(self,l):
        """Parse standard format continuation data"""

        ops = ["<=",">=","=","<",">","~"]

        def beforeop(ss):
            ns = str(ss)
            for o in ops: ns = ns.split(o)[0]
            return ns

        def nextop(ss):
            for o in ops:
                if ss[:len(o)] == o: return o
            return None

        def is_xreflist(ss):
            refels = [s.strip() for s in ss.split(",")]
            for s in refels[:1]:
                if len(s) < 6 or s.isdigit() or not s.isalnum():
                    return False
            return True

        for s in l[9:80].split("$"):
            if not s.strip(): continue
            s = s.replace(" EQ ","=").replace(" AP ","~").replace(" LT ","<").replace(" LE ","<=").replace(" GT ",">").replace(" GE ",">=").strip()
            quant = beforeop(s)
            s = s[len(quant):]
            qvals = []
            xref = None
            while len(s):
                o = nextop(s)
                if not o: break
                s = s[len(o):]
                val = beforeop(s)
                if quant != "XREF" and s and len(val)==len(s) and val[-1] == ")":
                    n = val.rfind('(')
                    xref = val[n+1:-2]
                    if not is_xreflist(xref): xref = None
                    else: val = val[:n]
                s = s[len(val):]
                qvals.append((o,val.strip()))
            self.xdata[quant] = (qvals,xref)

class ENSDF_I(ENSDF_Record):
    """Identification record"""
    def __init__(self, l):
        super().__init__(l)
        assert self.rectp == "    "
        self.DSID  = l[9:39].strip()
        self.DSREF = l[39:65].strip()
        self.PUB   = l[65:74].strip()
        self.EDATE = int(l[74:80]) if l[74:80].strip() else 0

    def addCont(self, l):
        assert not l[39:].strip()
        assert self.DSID[-1] == ','
        self.DSID += " " + l[9:39].strip()



class ENSDF_H(ENSDF_Record):
    """History record"""
    def __init__(self, l):
        super().__init__(l)
        assert self.rectp[1:] == " H "
        self.History = l[9:80]

    def addCont(self, l):
        self.History += '\n'+l[9:80]

class ENSDF_Q(ENSDF_Record):
    """Q-value record"""
    def __init__(self, l):
        super().__init__(l)
        assert self.rectp == "  Q "
        self.Qm, self.DQm = parse_stderr(l[9:19],  l[19:21])
        self.SN, self.DSN = parse_stderr(l[21:29], l[29:31])
        self.SP, self.DSP = parse_stderr(l[31:39], l[39:41])
        self.QA, self.DQA = parse_stderr(l[41:49], l[49:55])
        self.QREF = l[55:80].strip()

class ENSDF_X(ENSDF_Record):
    """Cross-reference record"""
    def __init__(self, l):
        super().__init__(l)
        assert self.rectp[:3] == "  X"
        self.DSSYM = l[8]
        self.DSID =  l[9:39].strip()
        assert not l[39:80].strip()

class ENSDF_C(ENSDF_Record):
    """Comment record"""
    def __init__(self, l):
        super().__init__(l)
        assert self.rectp[1] in "cCdDtT"
        self.RTYPE = self.rectp[2]
        self.PSYM  = self.rectp[3]
        self.TEXT  = l[9:80]

    def __repr__(self):
        return indent(self.TEXT," %s%s# "%(self.RTYPE,self.PSYM))

class ENSDF_P(ENSDF_Record):
    """Parent record"""
    def __init__(self, l):
        super().__init__(l)
        assert self.rectp[:3] == "  P"
        self.n = int(l[8]) if l[8] != ' ' else None
        self.E,  self.DE  = parse_stderr(l[9:19],l[19:21])
        self.J            = l[21:39].strip()
        self.T,  self.DT  = parse_unit_stderr(l[39:50],l[50:64])
        self.QP, self.DQP = parse_stderr(l[64:74], l[74:76])
        self.ION          = l[76:80].strip()

class ENSDF_N(ENSDF_Record):
    """Normalization record"""
    def __init__(self, l):
        super().__init__(l)
        assert self.rectp[:3] == "  N"
        self.n = int(l[8]) if l[8] != ' ' else None
        self.NR, self.DNR = parse_stderr(l[9:19],  l[19:21])
        self.NT, self.DNT = parse_stderr(l[21:29], l[29:31])
        self.BR, self.DBR = parse_stderr(l[31:39], l[39:41])
        self.NB, self.DNB = parse_stderr(l[41:49], l[49:55])
        self.NP, self.DNP = parse_stderr(l[55:62], l[62:64])
        assert not l[64:80].strip()

class ENSDF_PN(ENSDF_Record):
    """Product Normalization record"""
    def __init__(self, l):
        super().__init__(l)
        assert self.rectp == " PN "
        self.NRxBR, self.DNRxBR = parse_stderr(l[9:19],  l[19:21])
        self.NTxBR, self.DNTxBR = parse_stderr(l[21:29], l[29:31])
        # [31:41] unspecified
        self.NBxBR, self.DNBxBR = parse_stderr(l[41:49], l[49:55])
        self.NP,    self.DNP    = parse_stderr(l[55:62], l[62:64])
        # [64:76] unspecified
        self.C = l[76]
        self.OPT = int(l[77]) if l[77] != ' ' else None

class ENSDF_L(ENSDF_Record):
    """Level record"""
    def __init__(self, l):
        super().__init__(l)
        assert self.rectp[1:] == " L "
        self.E, self.DE = parse_stderr(l[9:19], l[19:21])
        self.J  = l[21:39].strip()
        T0 = l[39:49].strip()
        self.T, self.DT = (float("inf"),None) if T0 == 'STABLE' else parse_unit_stderr(T0, l[49:55])
        self.L = l[55:64].strip()
        try:    self.S, self.DS = parse_stderr(l[64:74], l[74:76])
        except: self.S, self.DS = l[64:76].strip(), None
        self.C  = l[76]
        self.MS = l[77:79].strip()
        self.Q  = l[79]

        self.delayed = []

    def __repr__(self):
        """Extended description"""
        s = super().__repr__()
        if self.delayed:
            s += "\nFed by"
            for d in self.delayed: s += '\n'+indent(str(d),'\t')
        return s

class ENSDF_B(ENSDF_Record):
     """Beta- record"""
     def __init__(self, l):
        super().__init__(l)
        assert self.rectp[1:] == " B "
        self.E, self.DE      = parse_stderr(l[9:19],  l[19:21])
        self.IB, self.DIB    = parse_stderr(l[21:29], l[29:31])
        # ... undefined
        self.LOGFT, self.DFT = parse_stderr(l[41:49], l[49:55])
        assert not l[55:66].strip()
        self.C  = l[66]
        self.UN = l[77:79]
        self.Q =  l[79]

class ENSDF_E(ENSDF_Record):
    """Beta+ or EC record"""
    def __init__(self, l):
        super().__init__(l)
        assert self.rectp[1:] == " E "
        self.E,  self.DE     = parse_stderr(l[9:19],  l[19:21])
        self.IB, self.DIB    = parse_stderr(l[21:29], l[29:31])
        self.IE, self.DIE    = parse_stderr(l[31:39], l[39:41])
        self.LOGFT, self.DFT = parse_stderr(l[41:49], l[49:55])
        self.TI, self.DTI    = parse_stderr(l[55:74], l[74:76])
        self.C  = l[76]
        self.UN = l[77:79]
        self.Q  = l[79]

class ENSDF_A(ENSDF_Record):
    """Alpha record"""
    def __init__(self, l):
        super().__init__(l)
        assert self.rectp == "  A "
        self.E, self.DE   = parse_stderr(l[9:19],  l[19:21])
        self.IA, self.DIA = parse_stderr(l[21:29], l[29:31])
        self.HF, self.DHF = parse_stderr(l[31:39], l[39:41])
        assert not t[41:74].strip()
        self.C = l[74]
        assert l[69:71] == '  '
        self.Q = l[79]

class ENSDF_D(ENSDF_Record):
    """(Delayed-)particle record"""
    def __init__(self, l):
        super().__init__(l)
        assert self.rectp[1] == " " and self.rectp[2] in ('D',' ')
        self.particle = self.rectp[2]
        self.E,  self.DE  = parse_stderr(l[9:19],  l[19:21])
        self.IP, self.DIP = parse_stderr(l[21:29], l[29:31])
        self.EI = optfloat(l[31:39])
        self.T, self.DT   = parse_stderr(l[39:49], l[49:55])
        self.L = l[55:64].strip()
        # 64:76 "Blank"; but not "must be blank"
        self.C    = l[76]
        self.COIN = l[77]
        # 78: "Blank"
        self.Q    = l[79]

class ENSDF_G(ENSDF_Record):
    """Gamma record"""
    def __init__(self, l):
        super().__init__(l)
        assert self.rectp[1:] == " G "
        self.E,  self.DE  = parse_stderr(l[9:19],  l[19:21])
        self.RI, self.DRI = parse_stderr(l[21:29], l[29:31])
        self.M = l[31:41].strip()
        self.MR, self.DMR = parse_stderr(l[41:49], l[49:55])
        self.CC, self.DCC = parse_stderr(l[55:62], l[62:64])
        self.TI, self.DTI = parse_stderr(l[64:74], l[74:76])
        self.C    = l[76]
        self.COIN = l[77]
        assert l[78] == ' '
        self.Q    = l[79]

class ENSDF_R(ENSDF_Record):
    """Reference record"""
    def __init__(self, l):
        super().__init__(l)
        assert self.rectp == "  R "
        self.KEYNUM    = l[9:17].strip()
        self.REFERENCE = l[17:80].strip()

class ENSDF_Nuclide(ENSDF_I):
    """Load information on one nuclide from file lines"""
    def __init__(self, iterlines):

        self.comments = []  # file general comments
        self.records  = {}  # records by type
        self.delayed = []   # unassigned delayed particles
        self.NUCID = None

        rectps = {  'A': ENSDF_A,
                    'B': ENSDF_B,
                    'E': ENSDF_E,
                    'G': ENSDF_G,
                    'H': ENSDF_H,
                    'L': ENSDF_L,
                    'N': ENSDF_N,
                    'Z': ENSDF_PN,
                    'P': ENSDF_P,
                    'Q': ENSDF_Q,
                    'R': ENSDF_R,
                    'X': ENSDF_X }

        for l in iterlines:

            if self.NUCID is None:
                super().__init__(l)
                continue

            if not l.strip(): break # end record
            l = l[:80] # strip trailing newline
            #print(l)

            rt = l[7]
            if rt == 'N' and l[6] == 'P': rt = 'Z' # differentiate ProdNorm from Norm records

            # comment records
            if l[6] != ' ' and l[6:8] != "PN":
                cset = self.comments if rt not in self.records else self.records[rt][-1].comments
                if l[5] == ' ': cset.append(ENSDF_C(l)) # start of new comment
                else: cset[-1].TEXT += '\n'+l[9:80]     # continuation of previous
                continue

            # delayed particles feeding levels
            if rt in ' D' and l[8] in 'NPDTA':
                lr = self.records['L'][-1].delayed if 'L' in self.records else self.delayed
                if l[5] == ' ': lr.append(ENSDF_D(l))
                else: lr[-1].addCont(l)
                continue

            # (non-comment) continuation records
            if l[5] != ' ':
                assert l[5] != '1'
                if rt == ' ': self.addCont(l) # ID continuation
                else: self.records[rt][-1].addCont(l)
                continue

            self.records.setdefault(rt,[])
            rec = rectps[rt](l)
            self.records[rt].append(rec)

            if self.NUCID != rec.NUCID and rt != 'P':
                print("*** Misplaced NUCID! ****\n***")
                print(l)
                assert False

    def __repr__(self):
        s = "------------------------------"
        s += "\nENSDF Information on '%s':\n"%self.NUCID
        for c in self.comments: s += '\n'+str(c)+'\n'
        for rt in self.records:
            s += "\n\n----- %s records -----\n"%rt
            for r in self.records[rt]: s += '\n'+indent(str(r),'\t')
        if self.delayed:
            s += "\n\n----- Unassigned delayed particles -----\n"
            for r in self.delayed: s += '\n'+indent(str(r),'\t')
        s += "\n______________________________\n"
        return s


if __name__=="__main__":
    parser = ArgumentParser()
    parser.add_argument("--db",     help="location of DB file")
    parser.add_argument("--load",   help="import input file to DB")
    options = parser.parse_args()

    if options.load:

        print("Loading", options.load)
        f = open(options.load,"r")

        while f:
            N = ENSDF_Nuclide(f)
            if N.NUCID is None: break
            print(N)
