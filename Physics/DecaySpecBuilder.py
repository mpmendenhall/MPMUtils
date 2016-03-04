#!/usr/bin/python3
"""@package DecaySpecBuilder
Builds decay generator specification files from ENSDF inputs"""

from ENSDF_Reader import *
from SMFile import *

class CETable:
    """Conversion electron data table gleaned from a gamma line entry"""
    
    def __init__(self, g):
        self.cedat = { }
        for c in "KLMNOP":
            # KC theoretical coefficient; EKC measured coefficient; CEK conversion intensity
            d = [None,]*5
            ce = g.xvals.get("E"+c+"C",None) # measured coefficient
            if ce and ce[0][0] == "=":
                d[0] = ce[0][1]
            ce = g.xvals.get(c+"C",None)     # theoretical coefficient
            if ce and ce[0][0] == "=":
                d[1] = ce[0][1]
            ce = g.xvals.get(c+"C+",None)    # theoretical lumped
            if ce and ce[0][0] == "=":
                d[2] = ce[0][1]
            ce = g.xvals.get("CE"+c,None)    # measured conversion intensity
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

    def add_sheet(self, s):
        """Add parsed ENSDF datasheet; assigns unique global level numbers"""
        self.sheets.append(s)
        for li in s.levidx:
            l = s.levels[li]
            levid = (l.nucZ, l.nucA)
            n = self.levelcounter.setdefault(levid, -1) + 1
            l.levelID = (l.nucA, l.nucZ, n)
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
        for s in self.sheets:
           for l in s.levels.values():
               ml = KVMap()
               ml.insert("nm", "%i.%i.%i"%l.levelID)
               ml.insert("E", l.E.tofloat())
               
               hl = -1 if l.T.vs == "STABLE" else l.T.tofloat()
               hl = 0 if hl is None else hl
               ml.insert("hl","%g"%hl)
               
               if l.J:
                   ml.insert("jpi",l.J)
               
               smf.insert("level", ml)
               
    def transitionList(self, smf):
        """Output transitions to SMFile"""
        for s in self.sheets:
            for l in s.levels.values():
                for g in l.gammas:
                    Ig = g.RI.tofloat()
                    if not Ig:
                        continue
                    
                    mg = KVMap()
                    mg.insert("from", "%i.%i.%i"%l.levelID)
                    mg.insert("to", "%i.%i.%i"%g.goesto.levelID)
                    mg.insert("Igamma", Ig)
                    
                    ces = CETable(g).getCE()
                    for c in ces:
                        mg.insert("CE_"+c, ces[c])
                            
                    smf.insert("gamma", mg)
        
if __name__=="__main__":
    basedir = "/home/mpmendenhall/Documents/PROSPECT/RefPapers/ENSDF/"
    
    DSB = DecaySpecBuilder()
    DSB.add_sheet( ENSDF_Reader(basedir + "ENSDF_Bi207.txt") )
    #DSB.add_sheet( ENSDF_Reader(basedir + "ENSDF_214Bi-214Po.txt") )
    #DSB.add_sheet( ENSDF_Reader(basedir + "ENSDF_214Po-210Pb.txt") )
    
    print(DSB.headercomments())
    smf = SMFile()
    DSB.levellist(smf)
    DSB.transitionList(smf)
    
    print(smf.toString())