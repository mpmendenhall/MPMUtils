#!/usr/bin/python3
"""@package DecaySpecBuilder
Builds decay generator specification files from ENSDF inputs"""

from ENSDF_ParsedDB import *
from SMFile import *
from copy import *

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

class CETable:
    """Conversion electron data table gleaned from a gamma line entry"""
    
    def __init__(self, xv):
        self.cedat = { }
        for c in "KLMNOP":
            # KC theoretical coefficient; EKC measured coefficient; CEK conversion intensity
            d = [None,]*5
            ce = xv.get("E"+c+"C",None) # measured coefficient
            if ce and ce[0][0] == "=":
                d[0] = ce[0][1]
            ce = xv.get(c+"C",None)     # theoretical coefficient
            if ce and ce[0][0] == "=":
                d[1] = ce[0][1]
            ce = xv.get(c+"C+",None)    # theoretical lumped
            if ce and ce[0][0] == "=":
                d[2] = ce[0][1]
            ce = xv.get("CE"+c,None)    # measured conversion intensity
            if ce and ce[0][0] == "=":
                d[3] = ce[0][1]
            self.cedat[c] = tuple(d)
            
    def getCE(self):
        ces = {}
        for k in self.cedat:
            ce = self.cedat[k]
            c = None
            for i in range(3):
                if ce[i]:
                    c = ce[i]
                    break
            if c:
                v,dv = parse_stderr(*c.split())
                rs = "%g"%v
                if dv:
                    rs += "~%g"%dv
                ces[k] = rs 
        return ces

class DecaySpecBuilder:
    """Builds decay generator specification files from ENSDF inputs"""
    
    def __init__(self,curs):
        self.curs = curs
        self.sheets = []        # loaded ENSDF datasheets
        self.parents = []       # collection of all parent specifiers in loaded cards
        self.joins = []         # joining points between multiple cards
        self.min_tx_prob = 0    # ignore transitions less likely than this
        
    def add_sheet(self, cid):
        """Add parsed ENSDF datasheet"""
        card = ParsedCard(self.curs,cid)
        print("\nAdding card:")
        card.display()
        
        # auto-join multiple branches from same parent
        card.shiftE = 0
        for p in card.parents:
            p.E = p.QP
            isJoined = False
            for p1 in self.parents:
                if p.mass == p1.mass and p.elem == p1.elem and p.J == p1.J:
                    card.shiftE = p1.card.shiftE + p1.E - p.E
                    self.joins.append((p,p1))
                    isJoined = True
                    print("Joining parents %s"%p)
                    break
            if isJoined: continue
            self.parents.append(p)
            
        self.sheets.append(card)
        
        

    def assign_level_names(self):
        """Assign output names to active levels"""
        self.levels = []
        for s in self.sheets:
            self.levels += s.levels.values()
        self.levels += self.parents
        self.levels.sort(key=(lambda l: l.E))
        isotcounts = {}
        for l in self.levels:
            a = l.mass
            z = TheElementNames.elNum(l.elem)
            i = isotcounts.setdefault((a,z), 0)
            l.outName = "%i.%i.%i"%(a,z,i)
            isotcounts[(a,z)] += 1
        for j in self.joins: j[0].outName = j[1].outName
            
    def headercomments(self):
        """Generate header comments block"""
        h =  "#########################################\n"
        h += "# Decay scheme generated from ENSDF data:\n#\n"
        for s in self.sheets:
            rID = s.idrec
            h += "# '%s' %s:"%(rID.DSID, rID.EDATE)
            for rH in s.history:
                if "CIT" in rH.xdata:
                     h += " %s"%rH.xdata["CIT"][0][1]
            h += "\n"
        return h

    def levellist(self, smf):
        """Output energy levels to SMFile"""
        self.assign_level_names()
        
        for l in self.levels:
        
            ml = KVMap()
            ml.insert("nm", l.outName)
            ml.insert("E", "%.2f"%(l.E + l.card.shiftE))

            try: hl = float(l.T)
            except: hl = 0
            if hl == float("inf"): hl = -1
            ml.insert("hl","%g"%hl)
               
            if l.J: ml.insert("jpi",l.J)
            
            smf.insert("level", ml)
               
    def transitionList(self, smf):
        """Output transitions to SMFile"""
        
        def maketx(tx):
            try:
                m = KVMap()
                m.insert("from", tx.fromlvl.outName)
                m.insert("to", tx.tolvl.outName)
                return m
            except: return None
         
        for s in self.sheets:
            
            for a in s.alphas:
                if a.Ialpha <= self.min_tx_prob: continue
                ma = maketx(a)
                if not ma: continue
                ma.insert("I", "%g"%a.Ibeta)
                smf.insert("alpha", ma)
                
            for b in s.betas:
                if b.Ibeta <= self.min_tx_prob: continue
                mb = maketx(b)
                if not mb: continue
                mb.insert("I", "%g"%b.Ibeta)
                if b.UN and b.UN[1:] == "U":
                   mb.insert("forbidden", b.UN[:1]) 
                smf.insert("beta", mb)

            for g in s.gammas:
                if g.Igamma <= self.min_tx_prob: continue
                
                mg = maketx(g)
                if not mg: continue
                mg.insert("Igamma", "%g"%g.Igamma)
                mg.insert("E", "%g"%g.E)
                
                ces = CETable(g.xdata).getCE()
                for c in ces: mg.insert("CE_"+c, ces[c])
                            
                smf.insert("gamma", mg)
                
            for e in s.ecapts:
                if self.min_tx_prob < e.Iec:
                    me = maketx(e)
                    if not me: continue
                    me.insert("I","%g"%e.Iec)
                    smf.insert("ecapt", me)
                if self.min_tx_prob < e.Ibeta:
                    me = maketx(e)
                    if not me: continue
                    me.insert("I","%g"%e.Ibeta)
                    smf.insert("beta", me)

if __name__=="__main__":
    datdir = "/home/mpmendenhall/Documents/ENSDF/"
    conn = sqlite3.connect(datdir+"ENSDF.db")
    conn.row_factory = sqlite3.Row # fast name-based access to columns
    curs = conn.cursor()
    
    DSB = DecaySpecBuilder(curs)
    
    if False: # example with two joined branches
        curs.execute("SELECT rowid FROM ENSDF_cards WHERE DSID LIKE '40K %'")
        for cid in curs.fetchall(): DSB.add_sheet(cid[0])
    elif True:
        curs.execute("SELECT rowid FROM ENSDF_cards WHERE DSID LIKE '207BI EC%'")
        for cid in curs.fetchall(): DSB.add_sheet(cid[0])
        
    print(DSB.headercomments())
    smf = SMFile()
    DSB.levellist(smf)
    
    #DSB.min_tx_prob = 1
    DSB.transitionList(smf)
    
    print(smf.toString())
