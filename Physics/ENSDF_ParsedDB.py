#!/usr/bin/python3
"""@package ENSDF_ParsedDB
Parsed interpretation layer for ENSDF cards pulled into database"""

from ENSDF_CardDB import *
import sys, traceback
import itertools
from bisect import bisect_left

def optfloat(s):
    s = s.strip()
    if not s: return None
    try: return float(s)
    except: return s

def parse_stderr(s1,s2):
    """Interpret number with standard error notation"""
    s1 = s1.strip()
    s2 = s2.strip()
    if not s1: return (None,None)
    
    try:
        v = float(s1)
    except:
        return (s1,None)
    
    if not s2: return (v,None)
    try:
        u = int(s2)
    except:
        return (v,s2)
    
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
    s1 = s1.strip()
    if s1[:1] == "(":
        return s1,s2
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
    
def upload_C_record(curs,l,pL):    
    """Parse and upload comment record"""
    assert l.rectp[1] in 'CDTcdt'
    assert pL
    t = l.txt
    RTYPE = l.rectp[2]
    PSYM = t[0]
    ts = t[1:].split("$")
    SYM = ts[0].strip() if len(ts) > 1 else None
    CTEXT = ts[1].strip() if SYM is not None else t[1:].strip()
    if SYM is None and t[10:11] == ' ' and not RTYPE == PSYM == ' ': # "old format" comments
        SYM = t[1:10].strip()
        if not SYM: SYM = None
        CTEXT = t[11:].strip()
    curs.execute("INSERT INTO comment_records VALUES (?,?,?,?,?)", (pL.lid, RTYPE, PSYM, SYM, CTEXT))

def upload_xdata(curs,l,pL):
    """Parse and upload continuation records"""
    assert pL                           # previous record being continued exists
    assert l.rectp[2] == pL.rectp[2]    # continuation of correct record type
    
    # special mode for H records
    if l.rectp[2] == "H":
        for s in l.txt[1:].split("$"):
            n = s.find("=")
            if n < 0: continue
            quant = s[:n].strip()
            val = s[n+1:].strip()
            curs.execute("INSERT INTO record_xdata VALUES (?,?,?,?,?)", (pL.lid, quant, "=", val, None))
        return
    
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
        
    for s in l.txt[1:].split("$"):
        if not s.strip():
            continue
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
        for q in qvals:
            curs.execute("INSERT INTO record_xdata VALUES (?,?,?,?,?)", (pL.lid, quant, q[0], q[1], xref))

class DBRecord:
    """Generic row loaded from DB into class variables"""
    def __init__(self, curs, tbl, rowid, loadall = False):
        self.table = tbl
        self.rowid = rowid
        curs.execute("SELECT * FROM %s WHERE rowid = ?"%tbl, (rowid,))
        row = curs.fetchone()
        self.dbrow = dict(row)
        self.__dict__.update(row)
        self.xdata = {}
        
        if hasattr(self, "line"):
            l = DBRecord(curs, "ENSDF_lines", self.line)
            self.mass = l.mass
            self.elem = l.elem
            self.rectp = l.rectp
            if loadall: self.load_extras(curs)
    
    def load_extras(self,curs):
        """Load associated continuation and comment data"""
        curs.execute("SELECT * FROM record_xdata WHERE line = ?", (self.line,))
        for r in curs.fetchall(): self.xdata.setdefault(r["quant"], []).append((r["op"],r["val"],r["refs"]))
        
        curs.execute("SELECT rowid FROM comment_records WHERE line = ?", (self.line,))
        self.comments = [ParsedCommentRecord(curs,r[0]) for r in curs.fetchall()]
        
    def isot(self):
        """Isotope identifier"""
        try: return (self.mass, self.elem)
        except: return None
    
    def repr_xdat(self):
        s = []
        for k,v in self.xdata.items():
            s.append("%s"%k + ",".join([str(u[0])+str(u[1]) for u in v]))
            if v[0][-1]: s[-1] += " (%s)"%v[0][-1]
        return "{ "+" | ".join(s)+" }"
    
    def repr_comments(self):
        return "\n".join([str(c) for c in self.comments])

    def __repr__(self):
        t = "< " + " | ".join(["%s:%s"%(k,v) for k,v in self.dbrow.items() if v not in [None,'',' '] and k != "line"])
        if self.xdata: t += " "+self.repr_xdat()
        return t + " >"
    
    def displayform(self):
        s = str(self)
        return s if not self.comments else s+"\n"+self.repr_comments()

class ParsedCommentRecord(DBRecord):
    def __init__(self, curs, cid):
        DBRecord.__init__(self,curs,"comment_records",cid,False)
    def __repr__(self):
        t = "/// "
        if self.RTYPE != ' ': t += "%s) "%self.RTYPE
        if self.SYM: t += "%s: "%self.SYM
        return t + "\n//  ".join(self.CTEXT.split("\n"))
    
    
    
    
    
    
def upload_A_record(curs,l):
    """Parse and upload Alpha record from raw line"""
    t = l.txt
    E,DE = parse_stderr(t[1:11], t[11:13])
    IA,DIA = parse_stderr(t[13:21], t[21:23])
    HF,DHF = parse_stderr(t[23:31], t[31:43])
    assert not t[33:68].strip()
    C = t[68:69]
    assert not t[69:71].strip()
    Q = t[71:72]
    wlid = l.withlevel.rowid if l.withlevel else None
    curs.execute("INSERT INTO alpha_records VALUES (?,?,?,?,?,?,?,?,?,?)", (l.lid,wlid,E,DE,IA,DIA,HF,DHF,C,Q))
    
def upload_B_record(curs,l):
    """Parse and upload Beta- record from raw line"""
    t = l.txt
    E,DE = parse_stderr(t[1:11], t[11:13])
    IB,DIB = parse_stderr(t[13:21], t[21:23])
    # ... undefined
    LOGFT,DFT = parse_stderr(t[33:41], t[41:47])
    assert not t[47:68].strip()
    C = t[68:69]
    UN = t[69:71]
    Q = t[71:72]
    wlid = l.withlevel.rowid if l.withlevel else None
    curs.execute("INSERT INTO beta_records VALUES (?,?,?,?,?,?,?,?,?,?,?)", (l.lid,wlid,E,DE,IB,DIB,LOGFT,DFT,C,UN,Q))

def upload_D_record(curs,l):
    """Parse and upload Delayed Particle record from raw line"""
    t = l.txt
    particle = t[:1]
    E,DE = parse_stderr(t[1:11], t[11:13])
    IP,DIP = parse_stderr(t[13:21], t[21:23])
    EI = optfloat(t[23:31]) 
    T,DT = parse_stderr(t[31:41],t[41:47])
    L = t[47:56].strip()
    # 56:68 "Blank"; but not "must be blank"
    C = t[68:69]
    COIN = t[69:70]
    # 70:71 "Blank"
    Q = t[71:72]
    wlid = l.withlevel.rowid if l.withlevel else None
    curs.execute("INSERT INTO dptcl_records VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?)", (l.lid,wlid,particle,E,DE,IP,DIP,EI,T,DT,L,C,COIN,Q))

def upload_E_record(curs,l):
    """Parse and upload Electron Capture / Beta+ record"""
    t = l.txt
    E,DE = parse_stderr(t[1:11], t[11:13])
    IB,DIB = parse_stderr(t[13:21], t[21:23])
    IE,DIE = parse_stderr(t[23:31], t[31:33])
    LOGFT,DFT = parse_stderr(t[33:41], t[41:47])
    TI,DTI = parse_stderr(t[47:66], t[66:68])
    C = t[68:69]
    UN = t[69:71]
    Q = t[71:72]
    wlid = l.withlevel.rowid if l.withlevel else None
    curs.execute("INSERT INTO ec_records VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)", (l.lid,wlid,E,DE,IB,DIB,IE,DIE,LOGFT,DFT,TI,DTI,C,UN,Q))
    
def upload_G_record(curs,l):
    """Parse and upload Gamma record from raw line"""
    t = l.txt
    E,DE = parse_stderr(t[1:11], t[11:13])
    RI,DRI = parse_stderr(t[13:21], t[21:23])
    M = t[23:33].strip()
    MR,DMR = parse_stderr(t[33:41],t[41:47])
    CC,DCC = parse_stderr(t[47:54],t[54:56])
    TI,DTI = parse_stderr(t[56:66],t[66:68])
    C = t[68:69]
    COIN = t[69:70]
    assert not t[70:71].strip()
    Q = t[71:72]
    wlid = l.withlevel.rowid if l.withlevel else None
    curs.execute("INSERT INTO gamma_records VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)", (l.lid,wlid,None,E,DE,RI,DRI,M,MR,DMR,CC,DCC,TI,DTI,C,COIN,Q))
    return DBRecord(curs, "gamma_records", curs.lastrowid)

def upload_H_record(curs,l):
    """Parse and upload History record from raw line"""
    curs.execute("INSERT INTO history_records VALUES (?)", (l.lid,))
    upload_xdata(curs,l,l)

def upload_I_record(curs,l):
    t = l.txt
    assert t[0]==' '
    DSID = t[1:31].strip()
    DSREF = t[31:57].strip()
    PUB = t[57:66].strip()
    EDATE = int(t[66:72]) if t[66:72].strip() else 0
    curs.execute("INSERT INTO identification_records VALUES (?,?,?,?,?)", (l.lid,DSID,DSREF,PUB,EDATE))
    
def upload_L_record(curs,l):
    """Parse and upload Level record from raw line""" 
    t = l.txt
    E,DE = parse_stderr(t[1:11], t[11:13])
    J = t[13:31].strip()
    T0 = t[31:41].strip()
    T,DT = (float("inf"),None)
    if T0 != "STABLE": T,DT = parse_unit_stderr(T0,t[41:47])
    L = t[47:56].strip()
    try:
        S,DS = parse_stderr(t[56:66], t[66:68])
    except:
        S,DS = t[56:68].strip(), None
    C = t[68:69]
    MS = t[69:71].strip()
    Q = t[71:72]
    curs.execute("INSERT INTO level_records VALUES (?,?,?,?,?,?,?,?,?,?,?,?)", (l.lid,E,DE,J,T,DT,L,S,DS,C,MS,Q))
    return DBRecord(curs, "level_records", curs.lastrowid)



def upload_P_record(curs,l):
    """Parse and upload Parent record from raw line"""
    t = l.txt
    n = int(t[0]) if t[:1].strip() else None
    E,DE = parse_stderr(t[1:11],t[11:13])
    J = t[13:31].strip()
    T,DT = parse_unit_stderr(t[31:41],t[41:56])
    QP,DQP = parse_stderr(t[56:66],t[66:68])
    ION = t[68:72].strip()
    curs.execute("INSERT INTO parent_records VALUES (?,?,?,?,?,?,?,?,?,?)", (l.lid, n, E, DE, J, T, DT, QP, DQP, ION))

def upload_Q_record(curs,l):
    """Parse and upload Q-value record from raw line"""
    t = l.txt
    Qm,DQm = parse_stderr(t[1:11], t[11:13])
    SN,DSN = parse_stderr(t[13:21], t[21:23])
    SP,DSP = parse_stderr(t[23:31], t[31:33])
    QA,DQA = parse_stderr(t[33:41], t[41:47])
    QREF = t[47:72].strip()
    curs.execute("INSERT INTO qval_records VALUES (?,?,?,?,?,?,?,?,?,?)", (l.lid,Qm,DQm,SN,DSN,SP,DSP,QA,DQA,QREF))

def upload_R_record(curs,l):
    """Parse and upload Reference record from raw line"""
    t = l.txt
    assert t[0] == ' '
    KEYNUM = t[1:9].strip()
    REFERENCE = t[9:72].strip()
    curs.execute("INSERT INTO reference_records VALUES (?,?,?)", (l.lid,KEYNUM,REFERENCE))

def upload_X_record(curs,l):
    t = l.txt
    DSSYM = t[0]
    DSID = t[1:31].strip()
    # [31:72] blank
    curs.execute("INSERT INTO xref_records VALUES (?,?,?)", (l.lid,DSSYM,DSID))
    
def upload_N_record(curs,l):
    """Parse and upload Normalization record from raw line"""
    t = l.txt
    n = int(t[0]) if t[:1].strip() else None
    NR,DNR = parse_stderr(t[1:11], t[11:13])
    NT,DNT = parse_stderr(t[13:21], t[21:23])
    BR,DBR = parse_stderr(t[23:31], t[31:33])
    NB,DNB = parse_stderr(t[33:41], t[41:47])
    NP,DNP = parse_stderr(t[47:54], t[54:56])
    assert not t[56:72].strip()
    curs.execute("INSERT INTO normalization_records VALUES (?,?,?,?,?,?,?,?,?,?,?,?)", (l.lid, n, NR,DNR, NT,DNT, BR,DBR, NB,DNB, NP,DNP))
    
def upload_PN_record(curs,l):
    """Parse and upload Production Normalization record from raw line"""
    t = l.txt
    assert t[0] == ' '
    NRxBR,DNRxBR = parse_stderr(t[1:11], t[11:13])
    NTxBR,DNTxBR = parse_stderr(t[13:21], t[21:23])
    # [23:33] unspecified
    NBxBR,DNBxBR = parse_stderr(t[33:41], t[41:47])
    NP,DNP = parse_stderr(t[47:54], t[54:56])
    # [56:68] unspecified
    C = t[68:69]
    OPT = int(t[69:70]) if t[69:70].strip() else None
    # [71:72] unspecified
    curs.execute("INSERT INTO prodnorm_records VALUES (?,?,?,?,?,?,?,?,?,?,?)", (l.lid,NRxBR,DNRxBR,NTxBR,DNTxBR,NBxBR,DNBxBR,NP,DNP,C,OPT))

parsed_tables = ["comment_records","history_records","identification_records","record_xdata","parent_records",
                "qval_records","normalization_records","prodnorm_records","level_records","alpha_records",
                "beta_records","dptcl_records","ec_records","gamma_records"]

def clearParsed(curs, cid):
    """Clear parsed data for card"""
    cmd0 = 'DELETE FROM %s WHERE rowid IN (SELECT %s.rowid FROM ENSDF_lines INNER JOIN %s ON (ENSDF_lines.rowid = %s.line) WHERE ENSDF_lines.card = ?)'
    for tbl in parsed_tables:
        cmd = cmd0%(tbl,tbl,tbl,tbl)
        curs.execute(cmd, (cid,))

def clearOrphaned(curs):
    """Clear parsed data referencing non-existent lines"""
    cmd0 = 'DELETE FROM %s WHERE line NOT IN (SELECT rowid FROM ENSDF_lines)'
    for tbl in parsed_tables:
        cmd = cmd0%tbl
        print(cmd)
        curs.execute(cmd)
        
def deleteCard(curs,cid):
    """Delete all data for card"""
    clearParsed(curs,cid)
    curs.execute("DELETE FROM ENSDF_lines WHERE card = ?", (cid,))
    curs.execute("DELETE FROM ENSDF_cards WHERE rowid = ?", (cid,))

def parseCard(curs, C):
    """Generate "parsed" information for card"""
    
    prevLine = None     # previous commentable/continuable line
    prevComment = None  # previous comment line, used to glom continuations
    levels = []         # levels, in order of loading
    isotLevels = {}     # levels, lists per isotope
    gammas = []         # gammas (id, from-id, energy)
    
    
    rectps = {  'A': upload_A_record,
                'B': upload_B_record,
                'D': upload_D_record,
                ' ': upload_D_record,
                'E': upload_E_record,
                'G': upload_G_record,
                'H': upload_H_record,
                'L': upload_L_record,
                'N': upload_N_record,
                'P': upload_P_record,
                'Q': upload_Q_record,
                'R': upload_R_record,
                'X': upload_X_record }
    
    for l in C.lines:
        try:
            if l.rectp[0] != ' ': # continuation data
                if l.rectp[1] not in ' P': # comment continuation
                    assert prevComment
                    prevComment.txt += "\n" + l.txt
                else: upload_xdata(curs,l,prevLine)
                continue
            elif l.rectp[1] not in ' P': # should be starting a non-continued comment
                if prevComment is not None: upload_C_record(curs, prevComment, prevComment.myline)
                prevComment = l
                prevComment.myline = prevLine
                continue
            elif l.rectp[1:] == "PN": upload_PN_record(curs,l)
            elif l.rectp == "   " and l.txt[0] == ' ':
                upload_I_record(curs,l)
            elif l.rectp[2] in rectps:
                if l.rectp[2] in " DEBAG":
                    l.withlevel = levels[-1] if levels else None
                rec = rectps[l.rectp[2]](curs,l)
                if l.rectp[2] == "G": gammas.append(rec)
                if l.rectp[2] == "L":
                    levels.append(rec)
                    isotLevels.setdefault(rec.isot(),[]).append(rec)
            else:
                print("Unparsed line:",l)
            prevLine = l
        except:
            print("Failed line:",l)
            raise
        
    if prevComment is not None: upload_C_record(curs, prevComment, prevComment.myline)
    
    # rowid-sorted level index
    levels = dict([(l.rowid,l) for l in levels])
    
    # Assign probable levels for gamma transitions
    isotEs= {}
    for k,ll in isotLevels.items():
        for l in ll:
            if type(l.E) != type(1.): l.E = float("inf")
        ll.sort(key=(lambda l: l.E))
        isotEs[k] = [l.E for l in ll]
    
    for g in gammas:
        if g.fromlvl is None: continue
        if type(g.E) != type(1.): continue
        isot = g.isot()
        if isot not in isotLevels: continue
        
        E = levels[g.fromlvl].E - g.E
        i = bisect_left(isotEs[isot], E)
        if i == 0:
            g.probto = isotLevels[isot][0].rowid
        elif i == len(isotEs[isot]):
            g.probto = isotLevels[isot][0][-1].rowid
        else:
            before = isotEs[isot][i-1]
            after = isotEs[isot][i]
            g.probto = isotLevels[isot][i if after - E < E - before else i-1].rowid
        
        curs.execute("UPDATE gamma_records SET probto = ? WHERE rowid = ?", (g.probto,g.rowid))
        dE = levels[g.probto].E - E
        #if dE > 10: print("Uncertain level assignment dE = %g / %g"%(dE,g.E))
        
###########################
###########################

def rep_num(x,dx):
    if type(x) == type(dx) == type(1.): return "%g~%g"%(x,dx)
    t = str(x)
    if dx is not None: return t+"("+str(dx)+")"
    return t

class ParsedIDRecord(DBRecord):
    def __init__(self, curs, cid):
        DBRecord.__init__(self,curs,"identification_records",cid,True)
    def __repr__(self):
        return "[ENSDF Card: %s to %s%s (%s %s)] (%i)"%(self.DSID, self.mass, self.elem, self.PUB, self.EDATE, self.card.cid)

class ParsedParentRecord(DBRecord):
    def __init__(self,curs,cid):
        DBRecord.__init__(self,curs,"parent_records",cid,True)
    def __repr__(self):
        t = "[Parent %s%s J=%s: E %s keV, halflife %s s"%(self.mass, self.elem, self.J, rep_num(self.E,self.DE), rep_num(self.T, self.DT))
        if self.QP: t += ", Qgs %s keV"%rep_num(self.QP,self.DQP)
        t += "]"
        if self.xdata: t += " " + self.repr_xdat()
        return t
    
class ParsedLevelRecord(DBRecord):
    def __init__(self,curs,cid):
        DBRecord.__init__(self,curs,"level_records",cid,True)
    def __repr__(self):
        t = "[%s%s J=%s: E=%s, halflife=%s]"%(self.mass, self.elem, self.J, rep_num(self.E,self.DE), rep_num(self.T,self.DT))
        if self.xdata: t += " " + self.repr_xdat()
        return t

class ParsedAlphrecord(DBRecord):
    def __init__(self,curs,cid):
        DBRecord.__init__(self,curs,"alpha_records",cid,True)
    def __repr__(self):
        t = "[Alpha E=%s, intensity=%s]"%(rep_num(self.E,self.DE), rep_num(self.IA,self.DIA))
        if self.xdata: t += " " + self.repr_xdat()
        return t
    
class ParsedBetaRecord(DBRecord):
    def __init__(self,curs,cid):
        DBRecord.__init__(self,curs,"beta_records",cid,True)
    def __repr__(self):
        t = "[Beta- Endpt=%s, intensity=%s%s]"%(rep_num(self.E,self.DE), rep_num(self.IB,self.DIB), " %s"%self.UN if self.UN.strip() else "")
        if self.xdata: t += " " + self.repr_xdat()
        return t

class ParsedGammaRecord(DBRecord):
    def __init__(self,curs,cid):
        DBRecord.__init__(self,curs,"gamma_records",cid,True)
    def __repr__(self):
        t = "[Gamma E=%s, RI=%s]"%(rep_num(self.E,self.DE), rep_num(self.RI,self.DRI))
        if self.xdata: t += " " + self.repr_xdat()
        return t

class ParsedECRecord(DBRecord):
    def __init__(self,curs,cid):
        DBRecord.__init__(self,curs,"ec_records",cid,True)
    def __repr__(self):
        t = "[EC/Beta+ "
        if self.E: t += "Emeas=%s, "%rep_num(self.E,self.DE)
        t += "IE=%s, IB=%s%s]"%(rep_num(self.IE,self.DIE), rep_num(self.IB,self.DIB), " %s"%self.UN if self.UN.strip() else "")
        if self.xdata: t += " " + self.repr_xdat()
        return t  

class ParsedCard:
    """ENSDF card information, parsed to object"""
    def __init__(self, curs, cid):
        
        def cardlines(tbl, tp=None):
            """select lines for this card from specified table"""
            curs.execute("SELECT rowid FROM %s WHERE line IN (SELECT rowid FROM ENSDF_lines WHERE card = ?) ORDER BY rowid"%tbl,(cid,))
            if tp is None: rs = [DBRecord(curs, tbl, r[0], True) for r in curs.fetchall()]
            else: rs = [tp(curs, r[0]) for r in curs.fetchall()]
            for r in rs: r.card = self
            return rs
        
        self.cid = cid
        self.idrec = cardlines("identification_records", ParsedIDRecord)
        assert len(self.idrec) == 1
        self.idrec = self.idrec[0]
        self.history = cardlines("history_records")
        self.refs = cardlines("reference_records")
        self.xrefs = cardlines("xref_records")
        
        self.qvals = cardlines("qval_records")
        self.parents = cardlines("parent_records", ParsedParentRecord)
        self.norms = cardlines("normalization_records")
        self.prodnorms = cardlines("prodnorm_records")
        
        self.levellist = cardlines("level_records", ParsedLevelRecord)
        self.levellist.sort(key=(lambda l: l.E))
        self.levels = dict([(l.rowid,l) for l in self.levellist]) # index levels for transitions
        # top and bottom level link points
        self.linkpts = list(self.parents)
        if self.levellist: self.linkpts.append(self.levellist[0])
        
        self.alphas = cardlines("alpha_records", ParsedAlphrecord)
        self.betas = cardlines("beta_records", ParsedBetaRecord)
        self.gammas = cardlines("gamma_records", ParsedGammaRecord)
        self.dptcls = cardlines("dptcl_records")
        self.ecapts = cardlines("ec_records", ParsedECRecord)
        
        # connect normalizations, parents
        if not self.prodnorms or len(self.norms) == len(self.prodnorms):
            for (i,nrm) in enumerate(self.norms):
                # determine composite normalizations
                try: nrm.NRxBR = nrm.NR * nrm.BR
                except: nrm.NRxBR = 1
                try: nrm.NRxBR = float(self.prodnorms[i].NRxBR)
                except: pass
                
                try: nrm.NBxBR = nrm.NB * nrm.BR
                except: nrm.NBxBR = 1
                try: nrm.NBxBR = float(self.prodnorms[i].NBxBR)
                except: pass
                
                for p in self.parents:
                    if p.n == nrm.n:
                        p.norm = nrm
                        if self.prodnorms: p.prodnorm = self.prodnorms[i]
        else:
            print("***Warning: uninterpreted normalizations****")

        # connect level information to transitions
        for txs in (self.alphas, self.betas, self.dptcls, self.ecapts):
            for a in txs:
                if a.tolvl: a.tolvl = self.levels[a.tolvl]
                if len(self.parents) == 1: a.fromlvl = self.parents[0]
        for g in self.gammas:
            if g.fromlvl: g.fromlvl = self.levels[g.fromlvl]
            if g.probto: g.tolvl = self.levels[g.probto]
            else: g.tolvl = None
            
        # calculate transition absolute rates
        nrm = self.norms[0] if len(self.norms) else None
        for a in self.alphas:
            try: a.Ialpha = a.IA * nrm.NBxBR
            except: a.Ialpha = 0
        for b in self.betas:
            try: b.Ibeta = b.IB * nrm.NBxBR
            except: b.Ibeta = 0
        for g in self.gammas:
            try: g.Igamma = g.RI * nrm.NRxBR
            except: g.Igamma = 0
        for e in self.ecapts:
            try: e.Ibeta = e.IB * nrm.NBxBR
            except: e.Ibeta = 0
            try: e.Iec = e.IE * nrm.NBxBR
            except: e.Iec = 0
            
    def display(self):
        print(self.idrec.displayform())
        for a in self.history: print(a.displayform())
        for a in self.parents: print(a.displayform())
        for a in self.norms: print(a.displayform())
        for a in self.prodnorms: print(a.displayform())
        for a in self.qvals: print(a.displayform())
        for a in self.refs: print(a.displayform())
        for a in self.xrefs: print(a.displayform())
        print("-"*80)
        for l in self.levellist[::-1]:
            print(l.displayform())
            for a in [a for a in self.alphas if a.tolvl == l]: print("\t"+a.displayform())
            for a in [a for a in self.betas if a.tolvl == l]: print("\t"+a.displayform())
            for a in [a for a in self.gammas if a.fromlvl == l]: print("\t"+a.displayform())
            for a in [a for a in self.ecapts if a.tolvl == l]: print("\t"+a.displayform())
            for a in [a for a in self.dptcls if a.tolvl == l]: print("\t"+a.displayform())

def find_cardsfor(curs,A,elem):
    """Find cards related to A,elem in identification line"""
    curs.execute("SELECT rowid FROM ENSDF_cards WHERE mass = ? AND elem LIKE ? ORDER BY rowid",(A,elem))
    return [r[0] for r in curs.fetchall()]

def find_as_parent(curs,A,elem):
    """Return card ID's pertaining to specified A,elem as parent"""
    curs.execute("SELECT DISTINCT card FROM ENSDF_lines,parent_records WHERE line = ENSDF_Lines.rowid AND mass = ? AND elem LIKE ?",(A,elem))
    return [r[0] for r in curs.fetchall()]

def daughters_in(curs,cid):
    """Return (A,elem) for levels in scheme"""
    curs.execute("SELECT DISTINCT mass,elem FROM ENSDF_lines,level_records WHERE line = ENSDF_Lines.rowid AND card = ?",(cid,))
    return [tuple(r) for r in curs.fetchall()]

if __name__ == "__main__":
    datdir = "/home/mpmendenhall/Documents/ENSDF/"
    conn = sqlite3.connect(datdir+"ENSDF.db")
    conn.row_factory = sqlite3.Row # fast name-based access to columns
    curs = conn.cursor()
    
    if True:
        #curs.execute("SELECT rowid FROM ENSDF_cards WHERE DSID LIKE '%219AT%'")
        #cids = [r[0] for r in curs.fetchall()]
        cids = find_as_parent(curs,219,"At")
        #cids = find_cardsfor(curs,219,"At")
        for cid in cids:
            print("\n\n",cid)
            ParsedCard(curs,cid).display()
    
    exit(0)
    
    #curs.execute('select rowid from ENSDF_cards WHERE 1 ORDER BY rowid')
    clearOrphaned(curs)
    curs.execute('SELECT rowid FROM ENSDF_cards WHERE rowid not in (SELECT card FROM identification_records,ENSDF_lines WHERE line=ENSDF_lines.rowid) ORDER BY ENSDF_cards.rowid')
    for cid in curs.fetchall():
        clearParsed(curs, cid[0])
        C = ENSDFCardData(curs,cid[0])
        try:
            if not cid[0]%100: print(cid[0])
            if cid[0] == 1000: curs.execute('analyze')
            parseCard(curs,C)
        except:
            print(C.to_text())
            traceback.print_exc(file=sys.stdout)
            #raise
            
    curs.execute('analyze')
    conn.commit()
    conn.close()

# TODO
# fix weird commenty-looking lines in:
# SELECT DISTINCT SYM FROM comment_records WHERE 1;
# SELECT DISTINCT L FROM level_records WHERE 1; -- energy-like entries?
