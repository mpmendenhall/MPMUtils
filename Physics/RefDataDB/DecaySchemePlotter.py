#! /bin/env python
"""@package DecaySchemePlotter
Draw decay schemes built from ENSDF database"""

from NucCanvas import NucCanvas
from ENSDF_ParsedDB import *
from DecaySpecBuilder import *

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

    #dchain = find_chain(curs,232,"U",{})

    # Radon chain, downstream from 1600y Radium
    skip = {16341,16734} # stop at 210Pb (22y)
    #dchain = find_chain(curs,226,"Ra",skip)
    dchain = find_chain(curs,222,"Rn",skip)
    #dchain = find_chain(curs,214,"Bi",skip)

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
