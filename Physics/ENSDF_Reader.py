#!/usr/bin/python3

from bisect import bisect_left

class ENSDF_Number:
    def __init__(self, vs, dvs, u = None):
        self.vs = vs.strip()    # value, string format
        self.dvs = dvs.strip()  # uncertainty, string formay
        self.uvals = {"M":60., "S":1., "US":1e-6, "PS":1e-12, "keV":1, "%":0.01, None:1}
        self.unit = u
        assert u in self.uvals
        self.v = self.dv = None

        if self.vs:
            vparts = self.vs.split()
            assert 1 <= len(vparts) <= 2
            self.v = float(vparts[0])
            if len(vparts) == 2:
                assert u is None
                self.unit = vparts[1]
            
    def tofloat(self):
        return self.v * self.uvals[self.unit] if self.v is not None else None
        
    def __repr__(self):
        s = ""
        if self.v is not None:
            s = "%g"%self.v
            if self.dv is not None:
                s += " ~ %g"%self.dv
        else:
            s = "%s(%s)"%(self.vs, self.dvs)
            if s == "()":
                s = "?"
        if self.unit is not None:
            s += " " + self.unit
        return s

def to_ENSDF_Number(s):
    p = s.split(" ")
    if len(p) == 1:
        return ENSDF_Number(p[0], "")
    elif len(p) == 2:
        return ENSDF_Number(p[0], p[1])
    elif len(p) == 3:
        return ENSDF_Number(p[0], p[1], p[2])
    else:
        return None

class ENSDF_Record:
    def __init__(self,l):
        self.l = l.strip()
        self.NUCID = l[:5]      # nucleus ID
        self.CONT = l[5:6]      # continuation marker
        self.C = l[6:7]         # comment marker
        assert self.C in " CDTctP" or not self.C
        self.RTYPE = l[7:8]     # record type
        self.num = l[8:9].strip()       # multi-parent identifier
        self.continuation = None        # continuation data
        self.comment = None             # comment
        
    def __repr__(self):
        return "<Record %s>"%self.l
    
    def parent_id(self):
        return (self.NUCID, self.num)
    
    def cont_repr(self):
        s = "%s"%self.continuation if self.continuation else ""
        s += " // %s"%self.comment if self.comment else ""
        return s
    
    def display(self):
        print(self)

class ENSDF_End_Record:
     def __init__(self,l):
         ENSDF_Record.__init__(self,l)
     def __repr__(self):
        return "<End ENSDF File>"

class ENSDF_Continuation_Record(ENSDF_Record):
    def __init__(self,l):
        ENSDF_Record.__init__(self,l)
        assert self.RTYPE in "LBEGH "
        assert self.C in "ct " and not self.num
        self.vals = {}
        if self.C != " ": # comment-type continuation
            return
        
        ops = ["=","<",">","<=",">=","~"]
        for s in l[9:80].split("$"):
            # replace alternate operator symbols
            s = s.replace(" EQ ","=").replace(" AP ","~").replace(" LT ","<").replace(" LE ","<=").replace(" GT ",">").replace(" GE ",">=")
            # split on operator tokens
            for o in ops:
                ss = s.split(o)
                if len(ss) == 2:
                    self.vals.setdefault(ss[0],[]).append((o,to_ENSDF_Number(ss[1])))
                    break
                
    def merge(self, C):
        for k in C.vals:
            l = self.vals.setdefault(k,[])
            l += C.vals[k]
        
    def __repr__(self):
        s = "<Cont"
        for k in self.vals:
            s += " "+k
            for v in self.vals[k]:
                s += " %s %s"%v
        return s+">"
    
class ENSDF_Comment_Record(ENSDF_Record):
    def __init__(self,l):
        ENSDF_Record.__init__(self,l)
        self.RTYPE = ""
        self.text = l[7:]
    def __repr__(self):
        return "<Comment:%s>"%(self.text)
    
class ENSDF_Identification_Record(ENSDF_Record):
    def __init__(self,l):
        ENSDF_Record.__init__(self,l)
        assert l[5:9] == "    "
        self.DSID = l[9:39].strip()     # Data set ID
        self.DSREF = l[39:65].strip()   # References to supporting publications
        self.PUB = l[65:74].strip()     # Publication information
        self.DATE = l[74:80].strip()    # Date of entry into ENSDF database
    def __repr__(self):
        return "<Identification: %s(%s) Ref = %s, Pub = %s, Date = %s>"%(self.NUCID, self.DSID, self.DSREF, self.PUB, self.DATE)

class ENSDF_History_Record(ENSDF_Record):
    def __init__(self,l):
        ENSDF_Record.__init__(self,l)
    def __repr__(self):
        return "<History %s>"%self.l
    
class ENSDF_Parent_Record(ENSDF_Record):
    def __init__(self,l):
        ENSDF_Record.__init__(self,l)
        self.E = ENSDF_Number(l[9:19],l[19:21],"keV")
        self.J = l[21:39].strip()
        self.T = ENSDF_Number(l[39:49],l[49:55])
        # ground-state Q value, keV
        self.QP = ENSDF_Number(l[64:74],l[74:76],"keV")
        self.ION = l[76:80].strip()
        self.norms = {}        # normalization records by daughter
        
    def __repr__(self):
        return "<Parent%s %s: E = %s, Q = %s, T = %s>"%(self.num, self.NUCID, self.E, self.QP, self.T)

    def display(self):
        print(self)
        for kn in self.norms:
            print("\t",self.norms[kn])
            
class ENSDF_Normalization_Record(ENSDF_Record):
    def __init__(self,l):
        ENSDF_Record.__init__(self,l)
        assert l[5] == " " and self.C == " " and l[64:].strip() == ""
        
        # Multiplier for rel. photon intensity to per 100 parent decays
        self.NR = ENSDF_Number(l[9:19],l[19:21])
        # Multiplier for rel. total transition intensity to per 100 parent decays 
        self.NT = ENSDF_Number(l[21:29],l[29:31])
        # Branching ratio multiplier converting from this decay branch to parent total
        self.BR = ENSDF_Number(l[31:39],l[39:41])
        # Multiplier for relative beta-, EC to intensities per 100 decays in branch
        self.NB = ENSDF_Number(l[41:49],l[49:51])
        # Multiplier from delayed-transition intensity to precursor decays
        self.NP = ENSDF_Number(l[55:62],l[62:64])
        
    def __repr__(self):
        return "<Normalization %s%s: NR = %s, NT = %s, BR = %s, NB = %s, NP = %s>"%(self.NUCID, self.num, self.NR, self.NT, self.BR, self.NB, self.NP)


class ENSDF_Level_Record(ENSDF_Record):
    def __init__(self,l):
        ENSDF_Record.__init__(self,l)
        assert self.C == " " and not self.num
        # level energy in keV
        self.E = ENSDF_Number(l[9:19], l[19:21], "keV")
        assert self.E.tofloat() is not None
        self.J = l[21:39].strip()      # spin/parity
        # half-life, with units
        self.T = ENSDF_Number(l[39:49],l[49:55])
        self.L = l[55:64].strip()      # angular momentum transfer
        # spectroscopic strength for record
        self.S = ENSDF_Number(l[64:74],l[74:76])
        self.Cfl = l[76:77]            # comment flag connecting to comment record
        self.MS = l[77:79].strip()     # metastable state label
        self.Q = l[79:80]              # questionable decay marker
        self.feeders = []              # beta, electron capture feeding this level
        self.gammas = []               # gamma decays from this line
            
    def __repr__(self):
        s = "<Level %s: E = %s"%(self.NUCID, self.E)
        if self.J:
             s += " (J=%s)"%self.J
        return s+" lifetime %s>"%self.T + self.cont_repr()
    
    def level_id(self):
        return (self.NUCID, self.E.tofloat())
    
    def display(self):
        print(self)
        for f in self.feeders:
            print("\tfrom ",f)
        for g in self.gammas:
            print("\t",g)
        
class ENSDF_Beta_Record(ENSDF_Record):
    def __init__(self,l):
        ENSDF_Record.__init__(self,l)
        assert self.C == " " and not self.num
        
        # endpoint energy [keV], if measured
        self.E = ENSDF_Number(l[9:19], l[19:21], "keV")
        # intensity value
        self.IB = ENSDF_Number(l[21:29], l[29:31])
        # Log ft value
        self.LOGFT = ENSDF_Number(l[41:49], l[49:55])
        
        self.C_ = l[76:77].strip()      # coincidence comment
        self.UN = l[77:79].strip()     # forbiddenness
        self.Q = l[79:80]              # questionable decay marker
        
    def __repr__(self):
        s = "<Beta %s:"%(self.NUCID)
        if self.E.v:
            s += " E = %s"%self.E
        return s + " Branch = %s>"%self.IB + self.cont_repr()

class ENSDF_EC_Record(ENSDF_Record):
    def __init__(self,l):
        ENSDF_Record.__init__(self,l)
        assert self.C == " " and not self.num
        
        # energy for electron capture to level [keV], if measured or deduced from measured beta+
        self.E = ENSDF_Number(l[9:19], l[19:21], "keV")    
        # intensity of beta+ branch
        self.IB = ENSDF_Number(l[21:29], l[29:31])
        # intensity of electron capture branch
        self.IE = ENSDF_Number(l[31:39], l[39:41])
        # Log ft value for (ec + beta+)
        self.LOGFT = ENSDF_Number(l[41:49], l[49:55])
        # total intensity
        self.TI = ENSDF_Number(l[64:74], l[74:76])
        
        self.C_ = l[76:77].strip()      # coincidence comment
        self.UN = l[77:79].strip()     # forbiddenness
        self.Q = l[79:80]              # questionable decay marker
        
    def __repr__(self):
        s = "<Ecapt %s:"%(self.NUCID)
        if self.E.v:
            s += " E = %s"%self.E
        return s + " Total Intensity = %s>"%self.TI  + self.cont_repr()


class ENSDF_Alpha_Record(ENSDF_Record):
    
    def __init__(self,l):
        ENSDF_Record.__init__(self,l)
        assert self.C == " " and not self.num
        
        # alpha energy in keV
        self.E = ENSDF_Number(l[9:19], l[19:21], "keV")    
        # intensity of alpha branch as percent of total alpha decay
        self.IA = ENSDF_Number(l[21:29], l[29:31], "%")
        # hindrance factor for alpha decay
        self.HF = ENSDF_Number(l[31:39], l[39:41])
        
        assert not l[41:76].strip()
        self.C_ = l[76:77].strip()      # coincidence comment
        assert not l[77:79].strip()
        self.Q = l[79:80]               # questionable decay marker
        
    def __repr__(self):
        s = "<Alpha %s: E = %s Intensity = %s Hindrance = %s>"%(self.NUCID, self.E, self.IA, self.HF)
        return s + self.cont_repr()


class ENSDF_Gamma_Record(ENSDF_Record):
    def __init__(self,l):
        ENSDF_Record.__init__(self,l)
        assert self.C == " " and not self.num
        
        # energy for gamma [keV]
        self.E = ENSDF_Number(l[9:19], l[19:21], "keV")
        assert self.E.tofloat()
        
        # relative photon intensity, per NORMALIZATION record
        self.RI = ENSDF_Number(l[21:29], l[29:31])
        self.M = l[31:41].strip()       # multipolarity of transition
        # mixing ratio
        self.MR = ENSDF_Number(l[41:49], l[49:55])
        # total conversion coefficient
        self.CC = ENSDF_Number(l[55:62], l[62:64])
        # relative transition intensity, per NORMALIZATION record
        self.TI = ENSDF_Number(l[64:74], l[74:76])
        
        self.C_ = l[76:77].strip()      # comment flag
        self.COIN = l[77:78].strip()   # confirmed by coincidence?
        self.Q = l[79:80]              # questionable decay marker
        
        # assigned later: level this goes to
        self.goesto = None
        
    def __repr__(self):
        s = "<Gamma %s:"%(self.NUCID)
        if self.goesto:
            s += " to (%s,%g)"%self.goesto
        s += " E = %s"%self.E
        return s + " Rel. photon intensity = %s>"%self.RI  + self.cont_repr()       
        
        
        
        
def parse_record(l):
    """Identify and return correct record type from text line"""
    
    if not l.strip():
        return ENSDF_End_Record(l)
    
    if len(l) < 7:
           return None
    
    if l[5] != " ":
        return ENSDF_Continuation_Record(l)
    if l[6] != " ":
        return ENSDF_Comment_Record(l)

    if len(l) < 8:
           return None
       
    if l[7] == "H":
        return ENSDF_History_Record(l)
    if l[7] == "P":
        return ENSDF_Parent_Record(l)
    if l[7] == "N":
        return ENSDF_Normalization_Record(l)
    if l[7] == "L":
        return ENSDF_Level_Record(l)
    if l[7] == "B":
        return ENSDF_Beta_Record(l)
    if l[7] == "E":
        return ENSDF_EC_Record(l)
    if l[7] == "A":
        return ENSDF_Alpha_Record(l)
    if l[7] == "G":
        return ENSDF_Gamma_Record(l)
    
    return ENSDF_Record(l)

class ENSDF_Reader:
    def __init__(self, fname):
        self.lines = [ l[:-1] for l in open(fname,"r").readlines()]
        
        currentLevel = None
        prevMain = None
        
        self.IDrecord = ENSDF_Identification_Record(self.lines[0])
        self.topcomments = []   # unassigned top-level comments
        self.history = []       # reverse-chrnological-order file history
        self.parents = { }      # parent records (probably only 1; at most 2)
        self.levels = { }       # levels in decay scheme, indexed by (NUCID, E)
        norms = { }             # normalization records (attached to parents)
        unassigned = [ ]        # radiation unassigned to level structure
        self.unknown = []       # unclassified records
        
        for l in self.lines[1:]:
            #print(l)
            r = parse_record(l)
            if not r:
                print("Unparsed line '%s'"%l)
                continue
            
            # check for end
            if not r.l:
                break
            
            # check for continuations
            if r.CONT != " " and r.RTYPE != "":
                assert prevMain
                if prevMain.continuation is None:
                    prevMain.continuation = r
                else:
                    prevMain.continuation.merge(r)
                continue
            
            # check for comments
            if r.C != " ":
                if prevMain:
                    prevMain.comment = r
                else:
                    self.topcomments.append(r)
                prevMain = r
                continue
                
            prevMain = r
            
            if r.RTYPE == "H":
                self.history.append(r)
            elif r.RTYPE == "P":
                self.parents[r.parent_id()] = r
            elif r.RTYPE == "N":
                norms[r.parent_id()] = r
            elif r.RTYPE == "L":
                currentLevel = r
                assert r.level_id() not in self.levels
                self.levels[r.level_id()] = r
            elif r.RTYPE in "BEAG":
                if currentLevel:
                    if r.RTYPE == "G":
                        currentLevel.gammas.append(r)
                    else:
                        currentLevel.feeders.append(r)
                else:
                    unassigned.append(r)
            else:
                # unprocessed record
                self.unknown.append(r)
                print(r)
        
        print("\n\n---------------------------");
        
        self.IDrecord.display()
        
        for c in self.topcomments:
            c.display()
        
        for h in self.history:
            h.display()
    
        # associate normalizations with parents
        for kn in norms:
            for kp in self.parents:
                if kn[1] == kp[1]:
                    self.parents[kp].norms[kn[0]] = norms[kn]
        for kp in self.parents:
            self.parents[kp].display()
        
        print("---------------------------");
        
        # levels indexed by energy, and gamma transition assignments
        self.levidx = list(self.levels.keys())
        self.levidx.sort()
        for kl in self.levidx:
            l = self.levels[kl]
            for g in l.gammas:
                dE = l.E.tofloat() - g.E.tofloat()
                g.goesto = self.nearest_level(l.NUCID, dE)
                g.goesDE = dE - g.goesto[1]
                if abs(g.goesDE) > 1:
                    print("*** Warning: level match discrepancy: ",g.goesDE)
            l.display()
            
        
        # additional uncategorized records
        print("---------------------------");
        for r in self.unknown:
            r.display()
        
        
    def nearest_level(self, nucid, E):
        nuclevs = [l[1] for l in self.levidx if l[0] == nucid]
        if not len(nuclevs):
            return None
        
        i = bisect_left(nuclevs, E)
        
        if i == 0:
            return (nucid, nuclevs[0])
        elif i == len(nuclevs):
            return (nucid, nuclevs[-1])

        before = nuclevs[i-1]
        after = nuclevs[i]
        Ee = after if after - E < E - before else before
        return (nucid, Ee)
        
        
        
if __name__=="__main__":
    ENSDF_Reader("/home/mpmendenhall/Documents/PROSPECT/RefPapers/ENSDF/ENSDF_214Bi-214Po.txt")
    