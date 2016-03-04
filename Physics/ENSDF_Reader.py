#!/usr/bin/python3
"""@package ENSDF_Reader
Reader for ENSDF files"""

from bisect import bisect_left

class ElementNames:
    """Element name/symbol lookup table"""
    def __init__(self, fname="ElementsList.txt"):
        self.dat = [(int(x[0]),x[1],x[2]) for x in [l.split() for l in open(fname).readlines()] if len(x)==3]
        self.byNum = dict([(e[0],e) for e in self.dat])
        self.bySymb = dict([(e[1].lower(),e) for e in self.dat])
        self.byName = dict([(e[2].lower(),e) for e in self.dat])
        
    def elNum(self, s):
        """Element Z by name or symbol, case-insensitive"""
        s = s.lower()
        n = self.bySymb.get(s,None)
        n = n if n else self.byName.get(s,None)
        return n[0] if n else None

TheElementNames = ElementNames()

class ENSDF_Number:
    """Numerical value, possibly with units and errorbars or limit, from ENSDF text"""
    
    uvals = {"Y":365.25636*24*3600, "D": 24*3600, "H":3600,
                "M":60., "S":1., "MS":1e-3, "US":1e-6, "PS":1e-12,
                "FS":1e-15, "AS":1e-18,
                "EV": 1e-3, "KEV":1., "MEV":1e3,
                "%":0.01, None:1}
            
    def __init__(self, vs, dvs, u = None):
        self.vs = vs.strip()    # value, string format
        self.dvs = dvs.strip()  # uncertainty, string formay

        self.unit = u
        assert u is None or u.upper() in self.uvals
        self.v = self.dv = None

        if self.vs:
            try:
                vparts = self.vs.split()
                assert 1 <= len(vparts) <= 2
                self.vs = vparts[0]
                self.v = float(vparts[0])
                if len(vparts) == 2:
                    assert u is None
                    self.unit = vparts[1]
            except:
                pass
            
        if self.dvs and self.dvs.isdigit():
            newdvs = list(self.vs)
            i = self.vs.find("E")-1
            if i < 0:
                i = len(newdvs)-1
            j = len(self.dvs)-1
            while i >= 0:
                if newdvs[i].isdigit():
                    newdvs[i] = self.dvs[j] if j >= 0 else '0'
                    j -= 1
                i -= 1
            self.dv = float(''.join(newdvs))
    
    def unit_mul(self):
        return self.uvals[self.unit.upper() if self.unit is not None else None]
    
    def tofloat(self):
        """Convert to float where interpretable as such"""
        return self.v * self.unit_mul() if self.v is not None else None
    
    def __iadd__(self,c):
        """+= assignemnt operator to value, where possible, in this items' units"""
        if self.v is not None:
            self.v += c
            self.vs = '' # remove original string representation
        return self
    
    def __repr__(self):
        s = ""
        if self.v is not None:
            s = "%g"%self.v
            if self.dv is not None:
                s += " ~ %g"%self.dv
        else:
            s = self.vs
            if self.dvs:
                s += "(%s)"%self.dvs
            if not self.vs and not self.dvs:
                s = "?"
                
        if self.unit is not None:
            s += " " + self.unit
        
        return s

def to_ENSDF_Number(s):
    """Parse string as ENSDF_Number"""
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
    """Base for all ENSDF record types"""
    
    def __init__(self,l):
        print(l)
        self.l = l.strip()
        self.NUCID = l[:5].strip() # nucleus ID
        if self.NUCID:
            self.nucA = int(''.join([c for c in self.NUCID if c.isdigit()]))
            self.nucZ = TheElementNames.elNum(''.join([c for c in self.NUCID if not c.isdigit()]))
        self.CONT = l[5:6]      # continuation marker
        self.C = l[6:7]         # comment marker
        assert self.C in " CDTctP" or not self.C
        self.RTYPE = l[7:8]     # record type
        self.CRTYPE = l[6:8]    # record type with comment flag, for continuations
        self.num = l[8:9].strip()       # multi-parent identifier
        self.xvals = {}                 # extra values from continuation
        self.comments = []              # comments entries
        
    def __repr__(self):
        return "<Record %s>"%self.l
    
    def parent_id(self):
        return (self.NUCID, self.num)
    
    def xvals_repr(self):
        s = ""
        if not self.xvals:
            return s
        s += "{"
        for k in self.xvals:
            for v in self.xvals[k]:
                s += " %s "%k + "%s %s"%v
        return s + " }"
    
    def display(self, d = 0):
        """Print to stdout at tab-offset depth"""
        print("\t"*d + str(self))
        xvr = self.xvals_repr()
        if xvr:
            print("\t"*d + xvr)
        for c in self.comments:
            c.display(d)

    def addContinuation(self, C):
        """Extend with a continuation record"""
        for k in C.vals:
            l = self.xvals.setdefault(k,[])
            l += C.vals[k]
            
    def complete(self):
        """Finish instantiation after all continuations added"""
        pass
    
class ENSDF_End_Record:
    """Blank line record at end of file"""
    def __init__(self,l):
        ENSDF_Record.__init__(self,l)
    def __repr__(self):
        return "<End ENSDF File>"

class ENSDF_Continuation_Record(ENSDF_Record):
    """Continuation record, plain text or $-delimited values"""
    def __init__(self,l):
        ENSDF_Record.__init__(self,l)
        assert self.RTYPE in "LBEGH " or self.C != " "
        assert self.CONT.isalnum() and self.CONT != "1"
        assert self.C in "CDTct " and not self.num
        self.vals = {}
        
        if self.C != " " or self.RTYPE=="H": # text-type continuation
            self.CTEXT = l[9:].strip()
            return
        
        # TODO standard allows multiple operators/values per key
        ops = ["=","<",">","<=",">=","~"]
        for s in l[9:80].split("$"):
            # replace alternate operator symbols
            s = s.replace(" EQ ","=").replace(" AP ","~").replace(" LT ","<").replace(" LE ","<=").replace(" GT ",">").replace(" GE ",">=")
            # split on operator tokens
            for o in ops:
                ss = s.split(o)
                if len(ss) == 2:
                    self.vals.setdefault(ss[0].strip(),[]).append((o,to_ENSDF_Number(ss[1])))
                    break
   
    def addContinuation(self, C):
        """Don't do this!"""
        assert False
        
    def __repr__(self):
        s = "<Cont"
        for k in self.vals:
            s += " "+k
            for v in self.vals[k]:
                s += " %s %s"%v
        if not self.vals:
            s += " " + self.l
        return s+">"

class ENSDF_Comment_Record(ENSDF_Record):
    """Comment text on another record; may refer to specific symbol"""
    def __init__(self,l):
        ENSDF_Record.__init__(self,l)
        self.PSYM = l[8:9]      # blank or symbol for comment on delayed particle
        self.CTEXT = l[9:80].strip() # comment text body
        
        # check for possible symbol comment refers to
        self.SYM = None
        if not self.PSYM == self.RTYPE == " " and "$" in self.CTEXT:
            ts = self.CTEXT.split("$")
            self.SYM = ts[0]
            self.CTEXT = ts[1]
            
    def addContinuation(self, C):
        self.CTEXT += "\n" + C.CTEXT
        
    def __repr__(self):
        s = "//"
        if self.SYM:
            s += " [%s] "%self.SYM
        else:
            s += "* "
        return s + self.CTEXT.replace("\n"," ")
    
    def display(self, d=0):
        s = ""
        for (n,l) in enumerate(self.CTEXT.split("\n")):
            s += "\t"*d + "//"
            if not n and self.SYM:
                s += "- [%s] "%self.SYM
            elif not n:
                s += "- "
            else:
                s += "  "
            s += l + "\n"
        print(s[:-1])

class ENSDF_Identification_Record(ENSDF_Record):
    """File identification record"""
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
    """History record with citations and comments on data"""
    def __init__(self,l):
        ENSDF_Record.__init__(self,l)
        assert self.C == " " and not self.num
        self.HIST = l[9:80]
        
    def __repr__(self):
        return "<History %s>"%self.NUCID
    
    def addContinuation(self, C):
        self.HIST += " " + C.CTEXT
    
    def complete(self):
        for h in self.HIST.split("$"):
            g = h.split("=")
            if len(g) != 2:
                continue
            self.xvals.setdefault(g[0].strip(),[]).append(("=",g[1].strip()))

class ENSDF_QValue_Record(ENSDF_Record):
    def __init__(self,l):
        ENSDF_Record.__init__(self,l)
        assert self.C == " " and not self.num
        # Total energy available for beta- decay, keV
        self.Qm = ENSDF_Number(l[9:19],l[19:21],"keV")
        # Neutron separation energy, keV
        self.SN = ENSDF_Number(l[21:29],l[29:31],"keV")
        # Proton separation energy, keV
        self.SP = ENSDF_Number(l[31:39],l[39:41],"keV")
        # Total energy available for alpha decay, keV
        self.QA = ENSDF_Number(l[41:49],l[49:55],"keV")
        self.QREF = l[55:80] # Reference citations
        
    def __repr__(self):
        return "<QValue %s>"%self.l

class ENSDF_XRef_Record(ENSDF_Record):
    """Cross-reference declaration record"""
    def __init__(self,l):
        ENSDF_Record.__init__(self,l)
        assert self.C == " "
        self.DSSYM = self.num # unique identifier for the dataset
        # Dataset ID, must match one of the DSIDs used
        self.DSID = l[9:39].strip()
       
    def __repr__(self):
        return "<XRef [%s] %s>"%(self.DSSYM, self.DSID)
    
class ENSDF_Parent_Record(ENSDF_Record):
    def __init__(self,l):
        ENSDF_Record.__init__(self,l)
        self.E = ENSDF_Number(l[9:19],l[19:21],"keV")   # energy above isotope ground state
        self.J = l[21:39].strip()                       # state Jpi
        self.T = ENSDF_Number(l[39:49],l[49:55])        # decay half-life
        self.QP = ENSDF_Number(l[64:74],l[74:76],"keV") # Q-value to decay scheme ground state
        self.ION = l[76:80].strip()                     # ionization state if ionized atom decay
        self.norms = {}                                 # normalization records indexed by daughter
        
    def __repr__(self):
        return "<Parent%s %s: E = %s, Q = %s, T = %s>"%(self.num, self.NUCID, self.E, self.QP, self.T)

    def display(self, d = 0):
        ENSDF_Record.display(self,d)
        for kn in self.norms:
            for n in self.norms[kn]:
                n.display(d+1)
            
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

class ENSDF_ProdNorm_Record(ENSDF_Record):
    def __init__(self,l):
        ENSDF_Record.__init__(self,l)
        assert self.C == "P" and not self.num
        
        # Multiplier for rel. photon intensity to per 100 parent decays
        self.NRxBR = ENSDF_Number(l[9:19],l[19:21])
        # Multiplier for rel. total transition intensity to per 100 parent decays 
        self.NTxBR = ENSDF_Number(l[21:29],l[29:31])
        # Multiplier for relative beta-, EC to intensities per 100 decays in branch
        self.NBxBR = ENSDF_Number(l[41:49],l[49:51])
        # Multiplier from delayed-transition intensity to precursor decays
        self.NP = ENSDF_Number(l[55:62],l[62:64])
        self.COMM = l[77:78]    # blank or 'C' to show comment from continuation records
        
    def __repr__(self):
        return "<ProdNorm %s: NRxBR = %s, NTxBR = %s, NBxBR = %s, NP = %s>"%(self.NUCID, self.NRxBR, self.NTxBR, self.NBxBR, self.NP)

class ENSDF_Level_Record(ENSDF_Record):
    """Energy level in decay scheme"""
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
        self.feeders = []              # beta, electron capture, alpha decays feeding this level from parent
        self.gammas = []               # gamma decays from this line
            
    def __repr__(self):
        s = "<Level %s: E = %s"%(self.NUCID, self.E)
        if self.J:
             s += " (J=%s)"%self.J
        return s+" lifetime %s>"%self.T
    
    def level_id(self):
        return (self.NUCID, self.E.tofloat())
    
    def display(self, d=0):
        ENSDF_Record.display(self,d)
        for f in self.feeders:
            f.display(d+1)
        for g in self.gammas:
            g.display(d+1)

class ParentLevel(ENSDF_Level_Record):
    """Decay scheme parent placed as top level in scheme"""
    def __init__(self,P):
        ENSDF_Record.__init__(self,P.l)
        self.E = P.QP
        self.J = P.J
        self.T = P.T
        self.feeders = []
        self.gammas = []

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
        return s + " Branch = %s>"%self.IB

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
        return s + " EC intensity = %s, beta+ = %s, Total Intensity = %s>"%(self.IE,self.IB,self.TI)

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
        return "<Alpha %s: E = %s Intensity = %s Hindrance = %s>"%(self.NUCID, self.E, self.IA, self.HF)

class ENSDF_DParticle_Record(ENSDF_Record):
    def __init__(self,l):
        ENSDF_Record.__init__(self,l)
        assert self.C == " "
        self.Particle = self.num # symbol for delayed particle
        
        # energy for delayed particle [keV]
        self.E = ENSDF_Number(l[9:19], l[19:21], "keV")
        # intensity as percent of total delayed particle emissions
        self.IP = ENSDF_Number(l[21:29], l[29:31], "%")
        # energy of level in "intermediate" nuclide
        self.EI = ENSDF_Number(l[31:39], "", "keV")
        # width of transition in keV
        self.T = ENSDF_Number(l[39:49], l[49:55], "keV")
        # angular momentum transfer of emitted particle
        self.L = l[55:64].strip()
        
        self.C_ = l[76:77].strip()      # comment flag
        self.COIN = l[77:78].strip()    # confirmed by coincidence?
        self.Q = l[79:80]               # questionable or unobserved placement
       
    def __repr__(self):
        return "<Delayed %s: E = %s, Intensity = %s, T = %s>"%(self.Particle, self.EI, self.IP, self.T)  
    
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
            s += " to (%s,%s)"%(self.goesto.NUCID, self.goesto.E)
        s += " E = %s"%self.E
        return s + " Rel. photon intensity = %s>"%self.RI     
        
class ENSDF_Reference_Record(ENSDF_Record):
    def __init__(self,l):
        ENSDF_Record.__init__(self,l)
        assert l[3:7] == "    " and self.num == " "
        self.MASS = int(l[0:3].strip()) # mass number
        self.KEYNUM = l[9:17].strip() # reference key number
        self.REFERENCE = l[17:80].strip() # abbreviated reference
       
    def __repr__(self):
        return "<Reference [%i] %s %s>"%(self.MASS, self.KEYNUM, self.REFERENCE)
    
class ENSDF_Reader:
    rectypes = {
            "H": ENSDF_History_Record,
            "Q": ENSDF_QValue_Record,
            "X": ENSDF_XRef_Record,
            "P": ENSDF_Parent_Record,
            "N": ENSDF_Normalization_Record,
            "L": ENSDF_Level_Record,
            "B": ENSDF_Beta_Record,
            "E": ENSDF_EC_Record,
            "A": ENSDF_Alpha_Record,
            "D": ENSDF_DParticle_Record,
            "G": ENSDF_Gamma_Record,
            "R": ENSDF_Reference_Record }
    
    def __init__(self, fname):
        self.lines = [ l[:-1] for l in open(fname,"r").readlines()]
        
        currentLevel = None
        prevRec = { }           # previous records of each type for continuation
        prevMain = None         # previous non-comment record
        
        self.IDrecord = ENSDF_Identification_Record(self.lines[0])
        self.history = []       # reverse-chrnological-order file history
        self.qvalue = None
        self.parents = { }      # parent records (probably only 1; at most 2)
        self.topcomments = []   # general comments referring to whole dataset
        self.levels = { }       # levels in decay scheme, indexed by (NUCID, E)
        norms = { }             # normalization records (attached to parents)
        self.unassigned = [ ]   # radiation unassigned to level structure
        self.refs = { }         # reference entries in special dataset
        self.unknown = [ ]      # unclassified records
        
        for l in self.lines[1:]:
            #print(l)
            r = self.parse_record(l)
            if not r:
                print("Unparsed line '%s'"%l)
                continue
            
            # check for end
            if not r.l:
                break
            
            # check for continuations
            if r.CONT != " ":
                assert prevRec[r.CRTYPE]
                prevRec[r.CRTYPE].addContinuation(r)
                continue
            
            if r.CRTYPE in prevRec:
                prevRec[r.CRTYPE].complete()
            prevRec[r.CRTYPE] = r
            
            # check for comments
            if r.C != " " and not (r.RTYPE == "N" and r.C == "P"):
                if prevMain:
                    prevMain.comments.append(r)
                else:
                    self.topcomments.append(r)
                continue
                
            prevMain = r
            
            if r.RTYPE == "H":
                self.history.append(r)
            elif r.RTYPE == "Q":
                assert not self.qvalue
                self.qvalue = r
            elif r.RTYPE == "P":
                self.parents[r.parent_id()] = r
            elif r.RTYPE == "N":
                norms.setdefault(r.parent_id(),[]).append(r)
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
                    self.unassigned.append(r)
            elif r.RTYPE == "R":
                self.refs[r.KEYNUM] = r
            else:
                # unprocessed record
                self.unknown.append(r)
                print(r)
        
        ####################################
        # post-read cleanup and finalization
        
        for r in prevRec.values():
            r.complete()
    
        # associate normalizations with parents
        for kn in norms:
            for kp in self.parents:
                if kn[1] == kp[1]:
                    self.parents[kp].norms[kn[0]] = norms[kn]
        
        # insert parents into levels list
        for P in self.parents.values():
            l = ParentLevel(P)
            self.levels[l.level_id()] = l
        
        # levels indexed by energy, and gamma transition assignments
        self.levidx = list(self.levels.keys())
        self.levidx.sort()
        for kl in self.levidx:
            l = self.levels[kl]
            for g in l.gammas:
                dE = l.E.tofloat() - g.E.tofloat()
                g.goesto = self.nearest_level(l.NUCID, dE)
                g.goesDE = dE - g.goesto.E.tofloat()
                if abs(g.goesDE) > 1:
                    print("*** Warning: level match discrepancy: ",g.goesDE)
        
        self.display()
            
    def display(self):
        """Display to stdout"""
        
        # header information
        print("\n\n---------------------------");
        
        self.IDrecord.display()
        
        for c in self.topcomments:
            c.display()
        
        for h in self.history:
            h.display()
    
        if self.qvalue:
            self.qvalue.display()
            
        for kp in self.parents:
            self.parents[kp].display()
        
        # assigned levels and transitions
        print("---------------------------");
        
        for kl in self.levidx:
            self.levels[kl].display()
        
        # additional uncategorized records
        print("---------------------------");
        for r in self.unassigned:
            r.display()
        for r in self.unknown:
            r.display()
        
    def parse_record(self,l):
        """Parse one record from a line, determining correct type"""
        
        if not l.strip():
            return ENSDF_End_Record(l)
        
        if len(l) < 7:
            return None
        
        if l[5] != " ":
            return ENSDF_Continuation_Record(l)
        if l[6] in "CDTct":
            return ENSDF_Comment_Record(l)

        if len(l) < 8:
            return None
        
        if l[6:8] == "PN":
            return ENSDF_ProdNorm_Record(l)
        return self.rectypes.get(l[7], ENSDF_Record)(l)

    def nearest_level(self, nucid, E):
        """Find nearest energy level of nucleus to specified energy"""
        
        nuclevs = [l for l in self.levidx if l[0] == nucid]
        if not len(nuclevs):
            return None
        
        i = bisect_left(nuclevs, (nucid,E))
        
        if i == 0:
            return self.levels[nuclevs[0]]
        elif i == len(nuclevs):
            return self.levels[nuclevs[-1]]

        before = nuclevs[i-1]
        after = nuclevs[i]
        return self.levels[after if after[1] - E < E - before[1] else before]

    def shift_E0(self, dE):
        """Shift all energy levels by specified amount [keV]"""
        for l in self.levels.values():
            l.E += dE


if __name__=="__main__":
    basedir = "/home/mpmendenhall/Documents/PROSPECT/RefPapers/ENSDF/"
    #ENSDF_Reader(basedir + "ENSDF_214Bi-214Po.txt")
    #ENSDF_Reader(basedir + "ENSDF_214Bi-210Tl.txt")
    #ENSDF_Reader(basedir + "ENSDF_210Tl-210Pb.txt")
    
    R = ENSDF_Reader(basedir + "ENSDF_214Po-210Pb.txt")
    R.shift_E0(1000.)
    R.display()
    
    #ENSDF_Reader(basedir + "ENSDF_Co60.txt")
    