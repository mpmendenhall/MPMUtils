#!/usr/bin/python
"""@package DecaySchemePlotter
Draw decay schemes built from ENSDF database"""

from ENSDF_ParsedDB import *
from DecaySpecBuilder import *
from AtomsDB import *
from math import *
from pyx import *
from pyx.color import rgb

class NucCanvas:
    """Utilities for drawing nuclear decay chains on A:Z canvas"""
    def __init__(self):
        self.c = canvas.canvas()
        text.set(mode="latex")
        text.preamble(r"\usepackage{mathtools}")
        self.nucs = set()       # isotopes (A,Z) in underlying list
        self.elnames =  ElementNames()
        self.dscale = 1.2       # overall drawing scale factor
        
        self.btri = path.path(path.moveto(0,0.2), path.lineto(0.25, -0.15),path.lineto(-0.25, -0.15),path.closepath())
        
        p0 = self.nucCenter(0,0)
        pZ = self.nucCenter(0,1)
        self.dz = (pZ[0]-p0[0], pZ[1]-p0[1])
    
    def toZ(self, elem): return self.elnames.elNum(elem)
    
    def addNuc(self,A,elem):
        n = (A, self.toZ(elem))
        self.nucs.add(n)
    
    def nucCenter(self,A,Z):
        """Drawing coordinates center for nuclide"""
        return -0.25*A*self.dscale, (Z-3./8.*A)*self.dscale # good for alpha/beta chain
    
    def drawNucs(self):
        for A,Z in self.nucs:
            x0,y0 = self.nucCenter(A,Z)
            self.c.stroke(path.rect(x0-0.45*self.dscale, y0-0.45*self.dscale, 0.9*self.dscale, 0.9*self.dscale))
            self.c.text(x0, y0, r"$\prescript{%i}{%i}{}$%s"%(A,Z,self.elnames.elSym(Z)), [text.halign.boxcenter])
                    
    def drawBeta(self,A,Z):
        """Draw marker for beta transition"""
        x0,y0 = self.nucCenter(A,Z+0.5)
        self.c.fill(self.btri,[rgb.blue, trafo.scale(self.dscale), trafo.rotate(180/pi * atan2(self.dz[0],self.dz[1])), trafo.translate(x0,y0)])
        
    def drawAlpha(self,A,Z):
        """Draw marker for alpha transition"""
        x0,y0 = self.nucCenter(A-0.5,Z-0.5)
        x1,y1 = self.nucCenter(A-3.3,Z-2)
        self.c.stroke(path.line(x0,y0,x1,y1),[style.linewidth.THick, rgb.red, deco.earrow([deco.filled([rgb.red])], size=self.dscale*0.3)])
        
class DecayScheme:
    
    def __init__(self, curs):
        self.curs = curs        # database cursor
        self.cards = { }        # loaded cards, indexed by database ID
        
    def get_card(self,cid):
        """Get card by ID, pulling from database if not already cached"""
        if cid not in self.cards: self.cards[cid] = ParsedCard(self.curs,cid)
        return self.cards[cid]
        
def find_chain(curs,A,elem,skip=set()):
    """Find relevant cards following decay of specified element"""
    search = {(A,elem)}
    found = dict()
    
    while len(search):
        A,elem = search.pop()
        cids = [cid for cid in find_as_parent(curs,A,elem) if cid not in skip]
        found[(A,elem)] = cids
        for c in cids:
            dts = daughters_in(curs,c)
            for d in dts:
                if d not in found: search.add(d)

    return found

def load_chain(curs,dchain):
    """Load cards for decay chain"""
    cs = {}
    for k in dchain:
        cs[k] = [ParsedCard(curs,cid) for cid in dchain[k]]
    return cs

if __name__=="__main__":
    
    datdir = "/home/mpmendenhall/Documents/ENSDF/"
    conn = sqlite3.connect(datdir+"ENSDF.db")
    conn.row_factory = sqlite3.Row # fast name-based access to columns
    curs = conn.cursor()
    
    #D = DecayScheme(conn.cursor())
    #print(find_chain(curs,235,"U"))
    
    skip = {16607,16608,16972,16440}
    skip.add(16975) # don't really need 215At either
    #dchain = find_chain(curs,227,"Ac",skip)
    
    dchain = find_chain(curs,232,"U",{})
    
    print(dchain)
    dchain = load_chain(curs,dchain)
    
    NC = NucCanvas()
    for n in dchain:
        NC.addNuc(*n)
    clist = []
    for k in dchain:
        for c in dchain[k]:
            print("\n\n")
            clist.append(c)
            if c.betas: NC.drawBeta(k[0],NC.toZ(k[1]))
            if c.alphas: NC.drawAlpha(k[0],NC.toZ(k[1]))
    NC.drawNucs()
    NC.c.writePDFfile("dchain")
    
    DSB = DecaySpecBuilder(curs)
    for c in clist: 
        if c.alphas or c.betas: DSB._add_card(c)
    
    #DSB.min_tx_prob = 5
    DSB.makeDecayspec()
