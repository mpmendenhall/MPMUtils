#!/usr/bin/python3
"""@package DecaySpecBuilder
Builds decay generator specification files from ENSDF inputs"""

from ENSDF_Reader import *
from SMFile import *
from copy import *

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
                rs = "%g"%c.tofloat()
                if c.dv:
                    rs += "~%g"%c.dv
                ces[k] = rs 
        return ces

class DecaySpecBuilder:
    """Builds decay generator specification files from ENSDF inputs"""
    
    def __init__(self):
        self.sheets = []        # loaded ENSDF datasheets
        self.levelcounter = { } # level number assignment count
        self.levidx = { }       # index of registered levels by level ID
        self.min_tx_prob = 0    # ignore transitions less likely than this
        
    def add_sheet(self, s, linklevel = None):
        """Add parsed ENSDF datasheet; assigns unique global level numbers"""
        
        # possible link into existing level scheme
        if linklevel:
            assert linklevel.levelID in self.levidx
            E0 = linklevel.E.tofloat()
            E1 = self.levidx[linklevel.levelID].E.tofloat()
            if E0 > E1:
                for sh in self.sheets:
                    sh.shift_E0(E0-E1)
            else:
               s.shift_E0(E1-E0) 
        
        self.sheets.append(s)
        
        for li in s.levidx:
            l = s.levels[li]
            levid = (l.nucA, l.nucZ)
            l.nIn = 0
            l.nOut = 0
            if hasattr(l,"levelID"):
                assert l.levelID[:2] == levid
                print("Linking level",l.levelID)
                continue
            n = self.levelcounter.setdefault(levid, -1) + 1
            l.levelID = (l.nucA, l.nucZ, n)
            self.levidx[l.levelID] = l
            self.levelcounter[levid] += 1
            print(l.levelID)

    def headercomments(self):
        """Generate header comments block"""
        h =  "#########################################\n"
        h += "# Decay scheme generated from ENSDF data:\n#\n"
        for s in self.sheets:
            rID = s.IDrecord
            h += "# '%s' %s:"%(rID.DSID, rID.DATE)
            for rH in s.history:
                if "CIT" in rH.xvals:
                     h += " %s"%rH.xvals["CIT"][0][1]
            h += "\n"
        return h

    def levellist(self, smf):
        """Output energy levels to SMFile"""
        for l in self.levidx.values():
            if not l.nOut and not l.nIn:
                continue
               
            ml = KVMap()
            ml.insert("nm", "%i.%i.%i"%l.levelID)
            ml.insert("E", "%.2f"%l.E.tofloat())
               
            hl = -1 if l.T.vs == "STABLE" else l.T.tofloat()
            if l.T.unit in ["MeV","MEV"]: hl = 0
            if hl is None: hl = 0
            ml.insert("hl","%g"%hl)
               
            if l.J:
                ml.insert("jpi",l.J)
            
            smf.insert("level", ml)
               
    def transitionList(self, smf):
        """Output transitions to SMFile"""
        for s in self.sheets:
            
            assert len(s.parents) == 1          # almost always true...
            P = tuple(s.parents.values())[0]    # parent for file
            
            for l in s.levels.values():
                
                # normalization record for daughter, if available
                nucnrm = P.norms.get(l.NUCID, None)
                nnBR = nucnrm.BR.tofloat() if nucnrm else None
                if nnBR is None: nnBR = 1.
                    
                for g in l.gammas:
                    Ig = g.RI.tofloat()
                    if Ig and nucnrm: Ig *= nucnrm.NR.tofloat() * nnBR
                    if not Ig or Ig <= self.min_tx_prob: continue
                
                    mg = KVMap()
                    mg.insert("from", "%i.%i.%i"%l.levelID)
                    mg.insert("to", "%i.%i.%i"%g.goesto.levelID)
                    mg.insert("Igamma", "%g"%Ig)
                    mg.insert("E", "%g"%g.E.tofloat())
                    
                    ces = CETable(g.xvals).getCE()
                    for c in ces: mg.insert("CE_"+c, ces[c])
                            
                    smf.insert("gamma", mg)
                    l.nOut += 1
                    g.goesto.nIn += 1
                    
                for f in l.feeders:
                    
                    mf = KVMap()
                    mf.insert("to", "%i.%i.%i"%l.levelID)
                    
                    # identify parent level
                    pnt = None
                    if f.RTYPE == "E": pnt = s.findParent(f.nucA, f.nucZ+1)
                    elif f.RTYPE == "B": pnt = s.findParent(f.nucA, f.nucZ-1)
                    elif f.RTYPE == "A": pnt = s.findParent(f.nucA+4, f.nucZ+2)
                    assert pnt
                    mf.insert("from", "%i.%i.%i"%pnt.asLevel.levelID)
                    
                    
                    # beta, electron capture branches normalization
                    NB = 1
                    if nucnrm:
                        nnNB = nucnrm.NB.tofloat()
                        if nnNB is None: nnNB = 1.
                        NB = nnNB * nnBR
                    
                    if f.RTYPE == "E":
                        doesEC = False
                        if f.IE.tofloat(): # electron capture
                            tprob = NB*f.IE.tofloat()
                            if tprob > self.min_tx_prob:
                                mfe = deepcopy(mf)
                                mfe.insert("I","%g"%tprob)
                                smf.insert("ecapt", mfe)
                                doesEC = True
                        if f.IB.tofloat(): # beta+
                            tprob = NB*f.IB.tofloat()
                            if tprob > self.min_tx_prob:
                                mfp = deepcopy(mf)
                                mfp.insert("I","%g"%tprob)
                                mfp.insert("positron","1")
                                smf.insert("beta", mfp)
                                doesEC = True
                        if not doesEC: continue
                    elif f.RTYPE == "B": # beta- decay
                        tprob = NB*f.IB.tofloat()
                        if tprob <= self.min_tx_prob: continue
                        mf.insert("I","%g"%tprob)
                        smf.insert("beta", mf)
                    elif f.RTYPE == "A": # alpha decay
                        tprob = f.IA.tofloat()*100
                        if tprob <= self.min_tx_prob: continue
                        mf.insert("I","%g"%tprob)
                        mf.insert("E","%g"%f.E.tofloat())
                        smf.insert("alpha", mf)
                        
                    l.nIn += 1
                    pnt.asLevel.nOut += 1
                    

if __name__=="__main__":
    basedir = "/home/mpmendenhall/Documents/PROSPECT/RefPapers/ENSDF/"
    
    DSB = DecaySpecBuilder()
    
    if True:
        # single-sheet case
        DSB.add_sheet( ENSDF_Reader(basedir + "ENSDF_12B-12C.txt") )
    
    if False:
        DSB.add_sheet( ENSDF_Reader(basedir + "ENSDF_214Bi-214Po.txt") )
        
        s2 = ENSDF_Reader(basedir + "ENSDF_214Po-210Pb.txt")
        pl = s2.findParent(214,84).asLevel
        pl.levelID = (214,84,0)
        DSB.add_sheet(s2, pl)
        
    if False:
        DSB.add_sheet( ENSDF_Reader(basedir + "ENSDF_152Eu-152Gd.txt") )
        s2 = ENSDF_Reader(basedir + "ENSDF_152Eu-152Sm.txt")
        pl = s2.findParent(152,63).asLevel
        pl.levelID = (152,63,0)
        DSB.add_sheet(s2, pl)
        
    print(DSB.headercomments())
    smf = SMFile()
    #DSB.min_tx_prob = 1
    DSB.transitionList(smf)
    DSB.levellist(smf)
        
    print(smf.toString())
