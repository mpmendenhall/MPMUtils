#!/bin/env python3

# rm $ENSDFDB; sqlite3 $ENSDFDB < ENSDF_DB_Schema.sql
# for f in ~/Data/ENSDF_181101/ensdf.*; do ./ENSDF_Reader.py --load $f; done


from textwrap import indent
from argparse import ArgumentParser
from AtomsDB import *
import pickle
import os
import traceback

class ENSDF_Parse_Exception(Exception):
    def __init__(self, message, l):
        super().__init__(message + "\n in line '%s'"%l)

class ENSDFDB:
    """Connection to and manipulation of ENSDF data DB"""

    def __init__(self,dbname = None):
        """Initialize with database filename to open"""
        if dbname is None: dbname = os.environ["ENSDFDB"]
        self.conn = sqlite3.connect(dbname)
        self.curs = self.conn.cursor()
        self.curs.execute("PRAGMA foreign_keys=ON")
        self.cache = {} # set to None to disable

    def find_entries(self, A, Z):
        """Return entry_id(s) by A,Z"""
        self.curs.execute("SELECT entry_id FROM ENSDF_entries WHERE A=? AND Z=?", (A,Z))
        return [r[0] for r in self.curs.fetchall()]

    def find_adopted(self, A, Z):
        """Find adopted levels/gammas entry_id for A,Z"""
        self.curs.execute("SELECT entry_id FROM ENSDF_entries WHERE A=? AND Z=? AND DSID LIKE ?", (A,Z,"ADOPTED LEVELS%"))
        return self.curs.fetchone()[0]

    def upload_entry(self, e):
        """Upload entry to DB"""
        self.curs.execute("DELETE FROM ENSDF_entries WHERE A=? AND Z=? AND DSID=?", (e.A, e.Z, e.DSID))
        self.curs.execute("INSERT INTO ENSDF_entries(A,Z,DSID,pcl) VALUES(?,?,?,?)", (e.A, e.Z, e.DSID, pickle.dumps(e)))

    def get_entry(self, eid):
        """Return (cached) entry from DB"""
        if self.cache is not None and eid in self.cache: return self.cache[eid]

        self.curs.execute("SELECT pcl FROM ENSDF_entries WHERE entry_id = ?", (eid,))
        e = pickle.loads(self.curs.fetchone()[0])
        if self.cache is not None: self.cache[eid] = e
        return e


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

def str_stderr(x,dx):
    symbs = {'GT': '>', 'LT': '<', 'GE': '>=', 'LE': '<=', 'AP': '~'}
    s = symbs.get(dx,'') + str(x)

    if dx is not None:
        try: s += " ~ %g"%dx
        except: s += " "+str(dx)
    return s

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
        self.xdata = {} # {quantity: ([(op1, val1), (op2, val2)], XREF) ... }
        self.comments = []  # comments on this record

        if line is None:
            self.NUCID = '     '
            self.rectp = '    '
        else:
            self.NUCID = line[:5]
            self.rectp = line[5:9]

    def RTYPE(self): return self.rectp[2]

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
        if self.rectp != "    ": raise ENSDF_Parse_Exception("expected [5:9] blank", l)
        self.DSID  = l[9:39].strip()
        self.DSREF = l[39:65].strip()
        self.PUB   = l[65:74].strip()
        self.EDATE = int(l[74:80]) if l[74:80].strip() else 0

    def addCont(self, l):
        if l[39:].strip(): raise ENSDF_Parse_Exception("Expected blank, got '%s'"%l[39:], l)
        if self.DSID[-1] != ',': raise ENSDF_Parse_Exception("Continued DSID should end in comma; was '%s'+'%s'"%(self.DSID, l[9:39].strip()), l)
        self.DSID += " " + l[9:39].strip()

    def printid(self):
        return self.NUCID.strip() + ": " + self.DSID



class ENSDF_H(ENSDF_Record):
    """History record"""
    def __init__(self, l):
        super().__init__(l)
        if self.rectp[1:] != " H ": raise ENSDF_Parse_Exception("expected [6:9] == ' H '", l)
        self.History = l[9:80]

    def addCont(self, l):
        self.History += '$'+l[9:80]

    def printid(self):
        """Short-form description"""
        return "(H): "+self.History

    def parse(self):
        n0 = 0
        hs = []
        for s in self.History.split("$"):
            i = s.find("=")
            if i==len(s):
                hs.append(("x%i"%n0, s[0].strip()))
                n0 += 1
            else:
                hs.append((s[:i].strip(), s[i+1:].strip()))
        self.items = dict(hs)
        del self.History

    def __repr__(self):
        """Extended description"""
        s = "--- History ---"
        for k,v in self.items.items(): s += "\n- %s:\t%s"%(k,v)
        return s


class ENSDF_Q(ENSDF_Record):
    """Q-value record"""
    def __init__(self, l):
        super().__init__(l)
        assert self.rectp == "  Q "
        self.Qm, self.DQm = parse_stderr(l[9:19],  l[19:21])    # total energy available for beta- decay of ground state
        self.SN, self.DSN = parse_stderr(l[21:29], l[29:31])    # neutron separation energy
        self.SP, self.DSP = parse_stderr(l[31:39], l[39:41])    # proton separation energy
        self.QA, self.DQA = parse_stderr(l[41:49], l[49:55])    # total energy available for alpha decay of ground state
        self.QREF = l[55:80].strip()

    def printid(self):
        """Short-form description"""
        qv = []
        if self.Qm is not None: qv.append("Q_beta = " + str_stderr(self.Qm, self.DQm) + " keV")
        if self.QA is not None: qv.append("Q_alpha = " + str_stderr(self.QA, self.DQA) + " keV")
        if self.SN is not None: qv.append("S_n = " + str_stderr(self.SN, self.DSN) + " keV")
        if self.SP is not None: qv.append("S_p = " + str_stderr(self.SP, self.DSP) + " keV")
        return "Q values: "+", ".join(qv)

class ENSDF_X(ENSDF_Record):
    """Cross-reference record"""
    def __init__(self, l):
        super().__init__(l)
        if self.rectp[:3] != "  X": raise ENSDF_Parse_Exception("expected [5:8] == '  X'", l)
        self.DSSYM = l[8]
        self.DSID =  l[9:39].strip()
        if l[39:80].strip(): raise ENSDF_Parse_Exception("expected [39:80] blank, got '%s'"%l[39:80], l)

    def printid(self):
        """Short-form description"""
        return "XRef %s: %s"%(self.DSSYM, self.DSID)


class ENSDF_C(ENSDF_Record):
    """Comment record"""
    def __init__(self, l):
        super().__init__(l)
        if self.rectp[1] not in "cCdDtT": raise ENSDF_Parse_Exception("invalid comment identifier '%s'"%self.rectp[1], l)
        self.RTYPE = self.rectp[2]
        self.PSYM  = self.rectp[3]
        self.TEXT  = l[9:80]

    def __repr__(self):
        return indent(self.TEXT,"#%s%s# "%(self.RTYPE,self.PSYM))

class ENSDF_P(ENSDF_Record):
    """Parent record"""
    def __init__(self, l):
        super().__init__(l)
        if self.rectp[:3] != "  P": raise ENSDF_Parse_Exception("Expected '  P', got '%s'"%self.rectp[:3], l)
        self.n = int(l[8]) if l[8] != ' ' else None
        self.E,  self.DE  = parse_stderr(l[9:19],l[19:21])
        self.J            = l[21:39].strip()
        self.T,  self.DT  = parse_unit_stderr(l[39:50],l[50:64])
        self.QP, self.DQP = parse_stderr(l[64:74], l[74:76])
        self.ION          = l[76:80].strip()

    def printid(self):
        """Short-form description"""
        return "Parent E = %s, T_h = %s"%(str_stderr(self.E, self.DE), str_stderr(self.T, self.DT))

class ENSDF_N(ENSDF_Record):
    """Normalization record"""
    def __init__(self, l):
        super().__init__(l)
        if self.rectp[:3] != "  N": raise ENSDF_Parse_Exception("Expected '  N', got '%s'"%self.rectp[:3], l)
        self.n = int(l[8]) if l[8] != ' ' else None
        self.NR, self.DNR = parse_stderr(l[9:19],  l[19:21])
        self.NT, self.DNT = parse_stderr(l[21:29], l[29:31])
        self.BR, self.DBR = parse_stderr(l[31:39], l[39:41])
        self.NB, self.DNB = parse_stderr(l[41:49], l[49:55])
        self.NP, self.DNP = parse_stderr(l[55:62], l[62:64])
        if l[64:80].strip(): raise ENSDF_Parse_Exception("Expected blank, got '%s'"%l[64:80], l)
        self.prodnorm = None

class ENSDF_PN(ENSDF_Record):
    """Product Normalization record"""
    def __init__(self, l):
        super().__init__(l)
        if self.rectp[:3] != " PN": raise ENSDF_Parse_Exception("Expected ' PN', got '%s'"%self.rectp[:3], l)
        self.n = int(l[8]) if l[8] != ' ' else None
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
        self.gammas = []
        self.fedby = []

        if l is None:
            self.E = self.DE = self.J = self.T = self.DT = self.S = self.dS = float("nan")
            self.J = self.L = ''
            self.C = self.Q = ' '
            self.MS = '  '
            return

        if self.rectp[1:] != " L ": raise ENSDF_Parse_Exception("Expected ' L ', got '%s'"%self.rectp[1:], l)
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

    def printid(self):
        """Short-form description"""
        s = "Level E = " + str_stderr(self.E, self.DE) + " keV, T_h = " + str_stderr(self.T, self.DT)
        xinfo = []
        if self.J: xinfo.append("J=%s"%self.J)
        if self.L: xinfo.append("L=%s"%self.L)
        if xinfo: s += ' ('+', '.join(xinfo)+')'
        if self.Q != ' ': s = '??? '+s
        return s

    def __repr__(self):
        """Extended description"""
        s = super().__repr__()
        for d in self.fedby: s += '\n<-'+indent(str(d),'\t')
        for d in self.gammas: s += '\n->'+indent(str(d),'\t')
        return s

class ENSDF_B(ENSDF_Record):
    """Beta- record"""
    def __init__(self, l):
        super().__init__(l)
        if self.rectp[1:] != " B ": raise ENSDF_Parse_Exception("Expected ' B ', got '%s'"%self.rectp[1:], l)
        self.E, self.DE      = parse_stderr(l[9:19],  l[19:21])
        self.IB, self.DIB    = parse_stderr(l[21:29], l[29:31])
        # ... undefined
        self.LOGFT, self.DFT = parse_stderr(l[41:49], l[49:55])
        if l[55:66].strip(): raise ENSDF_Parse_Exception("Expected blank, got '%s'"%l[55:66], l)
        self.C  = l[66]
        self.UN = l[77:79]
        self.Q =  l[79]

    def printid(self):
        """Short-form description"""
        s = "Beta E = " + str_stderr(self.E, self.DE) + " keV, branching " + str_stderr(self.IB, self.DIB) + " (log Ft = " + str_stderr(self.LOGFT, self.DFT) + ")"
        if self.Q != ' ': s = '??? '+s
        return s

class ENSDF_E(ENSDF_Record):
    """Beta+ or EC record"""
    def __init__(self, l):
        super().__init__(l)
        if self.rectp[1:] != " E ": raise ENSDF_Parse_Exception("Expected ' E ', got '%s'"%self.rectp[1:], l)
        self.E,  self.DE     = parse_stderr(l[9:19],  l[19:21])
        self.IB, self.DIB    = parse_stderr(l[21:29], l[29:31])
        self.IE, self.DIE    = parse_stderr(l[31:39], l[39:41])
        self.LOGFT, self.DFT = parse_stderr(l[41:49], l[49:55])
        self.TI, self.DTI    = parse_stderr(l[55:74], l[74:76])
        self.C  = l[76]
        self.UN = l[77:79]
        self.Q  = l[79]

    def printid(self):
        """Short-form description"""
        s = "EC/beta+ E = " + str_stderr(self.E, self.DE) + " keV"
        if self.IB: s += ", e+ " + str_stderr(self.IB, self.DIB)
        if self.IE: s += ", EC " + str_stderr(self.IE, self.DIE)
        if self.Q != ' ': s = '??? '+s
        return s

class ENSDF_A(ENSDF_Record):
    """Alpha record"""
    def __init__(self, l):
        super().__init__(l)
        if self.rectp != "  A ": raise ENSDF_Parse_Exception("Expected '  A ', got '%s'"%self.rectp, l)
        self.E, self.DE   = parse_stderr(l[9:19],  l[19:21])
        self.IA, self.DIA = parse_stderr(l[21:29], l[29:31])
        self.HF, self.DHF = parse_stderr(l[31:39], l[39:41])
        if l[41:74].strip(): raise ENSDF_Parse_Exception("Expected blank, got '%s'"%l[41:74], l)
        self.C = l[74]
        if l[69:71].strip(): raise ENSDF_Parse_Exception("Expected blank, got '%s'"%l[69:71], l)
        self.Q = l[79]

    def printid(self):
        """Short-form description"""
        s = "Alpha E = " + str_stderr(self.E, self.DE) + " keV, branching " + str_stderr(self.IA, self.DIA)
        if self.Q != ' ': s = '??? '+s
        return s

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

    def printid(self):
        """Short-form description"""
        s = "%s %s: "%("Delayed" if self.rectp == 'D' else "Prompt", self.particle)
        s += "E = %s, T_h = %s"%(str_stderr(self.E, self.DE), str_stderr(self.T, self.DT))
        if self.Q != ' ': s = '??? '+s
        return s

class ENSDF_G(ENSDF_Record):
    """Gamma record"""
    def __init__(self, l):
        super().__init__(l)
        if self.rectp[1:] != " G ": raise ENSDF_Parse_Exception("Expected ' G ', got '%s'"%self.rectp[1:], l)
        self.E,  self.DE  = parse_stderr(l[9:19],  l[19:21])
        self.RI, self.DRI = parse_stderr(l[21:29], l[29:31])
        self.M = l[31:41].strip()
        self.MR, self.DMR = parse_stderr(l[41:49], l[49:55])
        self.CC, self.DCC = parse_stderr(l[55:62], l[62:64])
        self.TI, self.DTI = parse_stderr(l[64:74], l[74:76])
        self.C    = l[76]
        self.COIN = l[77]
        if l[78] != ' ': raise ENSDF_Parse_Exception("Expected blank, got '%s'"%l[78], l)
        self.Q    = l[79]

    def printid(self):
        """Short-form description"""
        return "Gamma E = " + str_stderr(self.E, self.DE) + " keV, rel. intensity " + str_stderr(self.RI, self.DRI)


class ENSDF_R(ENSDF_Record):
    """Reference record"""
    def __init__(self, l):
        super().__init__(l)
        if self.rectp != "  R ": raise ENSDF_Parse_Exception("Expected '  R ', got '%s'"%self.rectp, l)
        self.KEYNUM    = l[9:17].strip()
        self.REFERENCE = l[17:80].strip()

    def printid(self):
        """Short-form description"""
        return "Ref [%s]: "%self.KEYNUM + self.REFERENCE






class ENSDF_Nuclide(ENSDF_I):
    """Load information on one nuclide from file lines"""
    def __init__(self, iterlines):

        self.history = []
        self.Qvalue = []
        self.xrefs = []
        self.parents = []
        self.norms = []
        self.refs = []

        self.comments = []  # file general comments
        self.levels = [ENSDF_L(None),]  # start with one unassigned level
        self.NUCID = None

        EN = ElementNames()

        current = None # current record for comments/continuations
        for l in iterlines:

            if self.NUCID is None:
                super().__init__(l)
                self.A = int(self.NUCID[:3].strip())
                self.Z = EN.elNum(self.NUCID[3:].strip())
                if self.Z is None and self.NUCID[3:] != '  ': self.Z = 100 + int(self.NUCID[3:])
                continue

            if not l.strip(): break # end record
            l = l[:80] # strip trailing newline
            #print(l)

            rt = l[7]
            if rt == 'N' and l[6] == 'P': rt = 'Z'      # differentiate ProdNorm from Norm records
            if rt == ' ' and l[8] in 'NPDTA': rt = 'D'  # special case for 'prompt-' D records

            # comment records
            if l[6] != ' ' and l[6:8] != "PN":
                if (current is None) or current.RTYPE() != rt: cset = self.comments
                else: cset = current.comments
                if l[5] == ' ': cset.append(ENSDF_C(l)) # start of new comment
                else: cset[-1].TEXT += '\n'+l[9:80]     # continuation of previous
                continue

            if self.NUCID != l[:5] and rt not in 'PHNZ':
                raise ENSDF_Parse_Exception("Misplaced NUCID '%s'"%l[:5], l)

            # (non-comment) continuation records
            elif l[5] != ' ':
                assert l[5] != '1'
                if rt == ' ': self.addCont(l) # ID continuation
                else: current.addCont(l)
                continue

            current = {'A': ENSDF_A, 'B': ENSDF_B, 'E': ENSDF_E, 'G': ENSDF_G, 'D': ENSDF_D,
                       'H': ENSDF_H, 'L': ENSDF_L, 'N': ENSDF_N, 'Z': ENSDF_PN,
                       'P': ENSDF_P, 'Q': ENSDF_Q, 'R': ENSDF_R, 'X': ENSDF_X }[rt](l)

            if rt == 'H': self.history.append(current)

            elif rt == 'Q': self.Qvalue.append(current)

            elif rt == 'X': self.xrefs.append(current)

            elif rt == 'P': self.parents.append(current)

            elif rt == 'N': self.norms.append(current)

            elif rt == 'Z':
                if not self.norms:
                    self.norms.append(current)
                else:
                    assert self.norms[-1].prodnorm is None
                    self.norms[-1].prodnorm = current

            elif rt == 'L': self.levels.append(current)

            elif rt == 'G': self.levels[-1].gammas.append(current)

            elif rt in 'ABED': self.levels[-1].fedby.append(current)

            elif rt == 'R': self.refs.append(current)

            else: print(l); assert False

        if not self.levels[0].fedby: self.levels = self.levels[1:]
        else: self.levels = self.levels[1:] + [self.levels[0],]
        for h in self.history: h.parse()

    def printid(self):
        s = super().printid()
        if self.parents: s += ' from ' + ' and '.join([p.NUCID.strip()+'(%s)'%str(p.E) for p in self.parents])
        return s

    def __repr__(self):
        s = "===========================\n= " + self.printid() + "\n==========================="
        for p in self.parents: s += '\n * '+str(p)
        for h in self.history: s += '\n\n' + str(h)
        s += '\n'
        for c in self.comments: s += '\n'+str(c)+'\n'
        for r in self.refs: s += '\n'+str(r)
        for x in self.xrefs: s += '\n'+str(x)
        s += "\n---------------------------\n"
        for q in self.Qvalue: s += '\n'+str(q)+'\n'
        for n in self.norms: s += '\n'+str(n)+'\n'

        if self.levels:
            s += "\n--- Levels ---\n"
            for l in self.levels: s += '\n'+indent(str(l),'\t')+'\n'

        s += "\n___________________________\n"

        return s


if __name__=="__main__":
    parser = ArgumentParser()
    parser.add_argument("--db",     help="location of DB file")
    parser.add_argument("--load",   help="import input file to DB")
    parser.add_argument("--quieter",   action="store_true", help="quieter bulk upload")
    options = parser.parse_args()

    if options.load:

        EDB = ENSDFDB(options.db)
        print("\n------- Loading", options.load)
        f = open(options.load,"r")

        while f:
            try:
                N = ENSDF_Nuclide(f)
                if N.NUCID is None: break
                EDB.upload_entry(N)
                if not options.quieter: print(N)
            except:
                traceback.print_exc()
                while next(f).strip(): continue


        EDB.conn.commit()

