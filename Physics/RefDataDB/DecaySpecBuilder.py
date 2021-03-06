#! /bin/env python3
"""@package DecaySpecBuilder
Builds decay generator specification files from ENSDF inputs"""

from ENSDF_ParsedDB import *
from SMFile import *
from copy import *
import numpy
from numpy import linalg
from AtomsDB import *

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
                cels = list(c.split())
                if len(cels) < 2: cels.append("")
                v,dv = parse_stderr(cels[0],cels[1])
                rs = "%g"%v
                if dv:
                    rs += "~%g"%dv
                ces[k] = rs
        return ces

class DecaySpecBuilder:
    """Builds decay generator specification files from ENSDF inputs"""

    def __init__(self,curs):
        self.curs = curs
        self.cards = []         # loaded ENSDF datasheets
        self.parents = []       # collection of all parent specifiers in loaded cards
        self.joins = []         # joining points (n0, n1, l0, l1) between cards n0,n1 levels l0,l1
        self.min_tx_prob = 0    # ignore transitions less likely than this
        self.filler_tx_prob = 1e-15     # assign this to zero-likelihood transitions as placeholder

    def bottom_join_levels(self,n0,n1):
        """Identify join points between card level lists with same energy scale"""
        levsets = {}
        for l in self.cards[n0].levellist + self.cards[n1].levellist:
            levsets.setdefault((l.mass,l.elem,l.J.replace("(","").replace(")","")), []).append(l)
        for ls in levsets.values():
            ls.sort(key=(lambda l: l.E))
            for i in range(len(ls)-1):
                if ls[i].card != ls[i+1].card and ls[i+1].E-ls[i].E < 0.5:
                    self.joins.append((n0,n1,ls[i],ls[i+1]))
                    print("Joining %s to %s"%(ls[i],ls[i+1]))

    def find_joins(self,n0,n1):
        """Identify level joins between cards n0 and n1"""
        c0 = self.cards[n0]
        c1 = self.cards[n1]

        for l0 in c0.linkpts:
            for l1 in c1.linkpts:
                if l0.mass == l1.mass and l0.elem == l1.elem and l0.J.strip("()") == l1.J.strip("()"):
                    if l0.E == l1.E == 0: self.bottom_join_levels(n0,n1)
                    else:
                        self.joins.append((n0,n1,l0,l1))
                        print("Joining %s to %s"%(l0,l1))



    def add_card(self, cid):
        """Add parsed ENSDF datasheet by DB number"""
        card = ParsedCard(self.curs,cid)
        self._add_card(card)

    def _add_card(self, card):
        """Add parsed ENSDF datasheet"""
        print("\nAdding card:")
        card.display()

        for p in card.parents:
            p.E = p.QP # assign energy from qvalue
            self.parents.append(p)
        self.cards.append(card)

    def calc_eshifts(self):
        """Calculate energy shifts to cards in unified join scheme"""
        for n1 in range(len(self.cards))[1:]:
            for n0 in range(n1):
                self.find_joins(n0,n1)

        nj = len(self.joins)
        m = numpy.matrix(numpy.zeros((nj+1, len(self.cards))))
        v = numpy.matrix(numpy.zeros((nj+1,1)))
        m[nj,0] = 1
        v[nj,0] = 0
        for (n,j) in enumerate(self.joins):
            m[n,j[0]] = 1
            m[n,j[1]] = -1
            v[n,0] = j[3].E - j[2].E

        print(m)
        dE = linalg.lstsq(m,v)[0]
        print(m*dE - v)

        for (n,c) in enumerate(self.cards): c.shiftE = dE[n,0]

    def assign_level_names(self):
        """Assign output names to active levels"""
        self.levels = []
        for s in self.cards:
            self.levels += s.levels.values()
        self.levels += self.parents
        self.levels.sort(key=(lambda l: l.E + l.card.shiftE))
        isotcounts = {}
        for l in self.levels:
            a = l.mass
            z = TheElementNames.elNum(l.elem)
            i = isotcounts.setdefault((a,z), 0)
            l.outName = "%i.%i.%i"%(a,z,i)
            l.isJoined = False
            isotcounts[(a,z)] += 1
        for j in self.joins:
            if j[3].outName > j[2].outName: j[3].outName = j[2].outName
            else: j[2].outName = j[3].outName
            j[3].isJoined = True
            # try to find half-life in one record
            if j[2].T is None: j[2].T = j[3].T
            if j[3].T is None: j[3].T = j[2].T

    def headercomments(self):
        """Generate header comments block"""
        h =  "#########################################\n"
        h += "# Decay scheme generated from ENSDF data:\n#\n"
        for s in self.cards:
            rID = s.idrec
            h += "# '%s' %s:"%(rID.DSID, rID.EDATE)
            for rH in s.history:
                if "CIT" in rH.xdata:
                     h += " %s"%rH.xdata["CIT"][0][1]
            h += "\n"
        return h

    def levellist(self, smf):
        """Output energy levels to SMFile"""
        self.calc_eshifts()
        self.assign_level_names()

        for l in self.levels:

            if l.isJoined: continue

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

        for s in self.cards:

            for a in s.alphas:
                if a.Ialpha < self.min_tx_prob: continue
                if a.Ialpha < self.filler_tx_prob: a.Ialpha = self.filler_tx_prob
                ma = maketx(a)
                if not ma: continue
                ma.insert("I", "%g"%a.Ialpha)
                smf.insert("alpha", ma)

            for b in s.betas:
                if b.Ibeta < self.min_tx_prob: continue
                if b.Ibeta < self.filler_tx_prob: b.Ibeta = self.filler_tx_prob
                mb = maketx(b)
                if not mb: continue
                mb.insert("I", "%g"%b.Ibeta)
                if b.UN and b.UN[1:] == "U":
                   mb.insert("forbidden", b.UN[:1])
                smf.insert("beta", mb)

            for g in s.gammas:
                if g.Igamma < self.min_tx_prob: continue
                if g.Igamma < self.filler_tx_prob: g.Igamma = self.filler_tx_prob

                mg = maketx(g)
                if not mg: continue
                mg.insert("Igamma", "%g"%g.Igamma)
                mg.insert("E", "%g"%g.E)

                ces = CETable(g.xdata).getCE()
                for c in ces: mg.insert("CE_"+c, ces[c])

                smf.insert("gamma", mg)

            for e in s.ecapts:
                if self.min_tx_prob <= e.Iec:
                    if e.Iec < self.filler_tx_prob: e.Iec = self.filler_tx_prob
                    me = maketx(e)
                    if not me: continue
                    me.insert("I","%g"%e.Iec)
                    smf.insert("ecapt", me)
                if self.min_tx_prob <= e.Ibeta:
                    if e.Ibeta < self.filler_tx_prob: e.Ibeta = self.filler_tx_prob
                    me = maketx(e)
                    if not me: continue
                    me.insert("I","%g"%e.Ibeta)
                    smf.insert("beta", me)

    def makeDecayspec(self):
        smf = SMFile()
        self.levellist(smf)
        self.transitionList(smf)
        print("\n\n")
        print(self.headercomments())
        print(smf.toString())


if __name__=="__main__":
    datdir = "/home/mpmendenhall/Documents/ENSDF/"
    conn = sqlite3.connect(datdir+"ENSDF.db")
    conn.row_factory = sqlite3.Row # fast name-based access to columns
    curs = conn.cursor()

    DSB = DecaySpecBuilder(curs)
    def findLike(s):
        curs.execute("SELECT rowid FROM ENSDF_cards WHERE DSID LIKE ?",(s,))
        return [r[0] for r in curs.fetchall()]

    if False: # example with two joined branches
        curs.execute("SELECT rowid FROM ENSDF_cards WHERE DSID LIKE '40K %'")
        for cid in curs.fetchall(): DSB.add_card(cid[0])
    elif False:
        curs.execute("SELECT rowid FROM ENSDF_cards WHERE DSID LIKE '207BI EC%'")
        for cid in curs.fetchall(): DSB.add_card(cid[0])


    #DSB.min_tx_prob = 5
    DSB.makeDecayspec()

