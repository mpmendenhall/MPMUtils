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
    PSYM = t[0]
    ts = t[1:].split("$")
    SYM = ts[0].strip() if len(ts) > 1 else None
    CTEXT = ts[1].strip() if SYM is not None else t[1:].strip()
    curs.execute("INSERT INTO comment_records VALUES (?,?,?,?)", (pL.lid, PSYM, SYM, CTEXT))

def upload_xdata(curs,l,pL):
    """Parse and upload continuation records"""
    assert pL                           # previous record being continued exists
    assert l.rectp[2] == pL.rectp[2]    # continuation of correct record type
    
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
    def __init__(self, curs, tbl, rowid):
        self.table = tbl
        self.rowid = rowid
        curs.execute("SELECT * FROM %s WHERE rowid = ?"%tbl, (rowid,))
        row = curs.fetchone()
        self.__dict__.update(row)
        
        if hasattr(self, "line"):
            l = DBRecord(curs, "ENSDF_lines", self.line)
            self.mass = l.mass
            self.elem = l.elem
    
    def isot(self):
        """Isotope identifier"""
        try: return (self.mass, self.elem)
        except: return None

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


def clearParsed(curs, cid):
    """Clear parsed data for card"""
    cmd0 = 'DELETE FROM %s WHERE rowid IN (SELECT %s.rowid FROM ENSDF_lines INNER JOIN %s ON (ENSDF_lines.rowid = %s.line) WHERE ENSDF_lines.card = ?)'
    for tbl in ["comment_records","record_xdata","parent_records","level_records","alpha_records","beta_records","dptcl_records","ec_records","gamma_records"]:
        cmd = cmd0%(tbl,tbl,tbl,tbl)
        curs.execute(cmd, (cid,))
        
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
                'E': upload_E_record,
                'G': upload_G_record,
                'L': upload_L_record,
                'P': upload_P_record  }
    
    for l in C.lines:
        try:
            if l.rectp[0] != ' ': # continuation data
                if l.rectp[1] not in ' P': # comment continuation
                    assert prevComment
                    prevComment.txt += "\n" + l.txt
                else: upload_xdata(curs,l,prevLine)
            elif l.rectp[1] not in ' P':
                if prevComment is not None: upload_C_record(curs, prevComment, prevLine)
                prevComment = l
            elif l.rectp[2] in rectps:
                if l.rectp[2] in "DEBAG":
                    l.withlevel = levels[-1] if levels else None
                rec = rectps[l.rectp[2]](curs,l)
                if l.rectp[2] == "G": gammas.append(rec)
                if l.rectp[2] == "L":
                    levels.append(rec)
                    isotLevels.setdefault(rec.isot(),[]).append(rec)
            prevLine = l
        except:
            print("Failed line:",l)
            raise
        
    if prevComment is not None: upload_C_record(curs, prevComment, prevLine)
    
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
        if dE > 10: print("Uncertain level assignment dE = %g / %g"%(dE,g.E))
        
if __name__ == "__main__":
    datdir = "/home/mpmendenhall/Documents/ENSDF/"
    conn = sqlite3.connect(datdir+"ENSDF.db")
    conn.row_factory = sqlite3.Row # fast name-based access to columns
    curs = conn.cursor()
    
    curs.execute('select rowid from ENSDF_cards WHERE 1 ORDER BY rowid')
    for cid in curs.fetchall():
        clearParsed(curs, cid[0])
        C = ENSDFCardData(curs,cid[0])
        try:
            print(cid[0])
            parseCard(curs,C)
        except:
            print(C.to_text())
            raise
            
    conn.commit()
    conn.close()

# TODO
# fix weird commenty-looking lines in:
# SELECT DISTINCT SYM FROM comment_records WHERE 1;
# energies in parent.L ?? ... NP probably
