#! /bin/env python
"""@package NucCanvas
Draw table-of-isotopes figures"""

from AtomsDB import *
from math import *
from pyx import *
from pyx.color import rgb, hsb

text.set(cls=text.LatexRunner)
text.preamble(r"\usepackage{isotope}")

class NucState:
    """Nuclear state with drawing instructions"""
    def __init__(self, A, Z, L = None, w = 1):
        self.A = A
        self.Z = Z
        self.L = L
        self.w = w
        self.HL = None  # halflife
        self.baseColor = rgb.black
        self.alphamin = 0.10
        self.dx = 0
        self.dy = 0
        self.frameStyle = []

    def idx(self): return (self.A, self.Z, self.L)

    def draw(self, NC):
        """Draw to NucCanvas"""
        x0,y0 = NC.nucCenter(self.A, self.Z)
        alpha = color.transparency(1. - self.w)
        d = 0.9
        if self.L: d -= 0.05*self.L
        fc = NC.HLcolor(self.HL) if self.HL else self.baseColor
        NC.stroke(path.rect(x0+(self.dx-0.5*d)*NC.dscale, y0+(self.dy-0.5*d)*NC.dscale, d*NC.dscale, d*NC.dscale),
                  self.frameStyle + [deformer.smoothed(radius=0.1*NC.dscale), fc, alpha])

        y0 -= 0.1*NC.dscale
        alpha = color.transparency(min(1.-self.alphamin, 1. - self.w))
        #symb = r"$^{%i}_{%i}$%s"%(self.A, self.Z, NC.elnames.elSym(self.Z))
        symb = "\isotope[%i][%i]{%s}"%(self.A, self.Z, NC.elnames.elSym(self.Z))
        NC.text(x0, y0, symb, [self.baseColor, text.halign.boxcenter, alpha])

class BaseTrans:
    """Generic transition"""
    def __init__(self,A,Z,w,c):
        self.A = A
        self.Z = Z
        self.w = w
        self.idchar = c
    def idx(self): return (self.idchar, self.A, self.Z)

class BetaTrans(BaseTrans):
    """Beta/positron transition"""
    def __init__(self, A, Z, w):
        super().__init__(A,Z,w,'b')
        self.btri = path.path(path.moveto(0,0.2), path.lineto(0.25, -0.15),path.lineto(-0.25, -0.15),path.closepath())


    def draw(self, NC):
        """Draw to NucCanvas"""
        isPositron = self.Z < 0
        A,Z = self.A,abs(self.Z)

        if isPositron:
            x0,y0 = NC.nucCenter(A, Z-0.5)
            c = rgb(1,0,1)
        else:
            x0,y0 = NC.nucCenter(A, Z+0.5)
            c = rgb.blue

        alpha = color.transparency(1. - self.w)
        p0 = NC.nucCenter(A, Z)
        pZ = NC.nucCenter(A, Z - (1 if isPositron else -1))
        thz = atan2(pZ[0]-p0[0], pZ[1]-p0[1]) * 180/pi
        NC.fill(self.btri, [c, alpha, trafo.scale(NC.dscale), trafo.rotate(thz), trafo.translate(x0,y0)])

class AlphaTrans(BaseTrans):
    """Alpha transition drawing instructions"""
    def __init__(self, A, Z, w):
        super().__init__(A,Z,w,'a')

    def draw(self, NC):
        """Draw to NucCanvas"""
        c = rgb.red
        alpha = color.transparency(1. - self.w)
        s = [style.linewidth.THick, c, alpha, deco.earrow([deco.filled([c])], size=NC.dscale*0.3)]

        if NC.dA[0] < 0:
            x0,y0 = NC.nucCenter(self.A,   self.Z,   0.3, -0.3)
            x1,y1 = NC.nucCenter(self.A-4, self.Z-2, -0.35, 0)
        else:
            x0,y0 = NC.nucCenter(self.A,   self.Z,   -0.3, -0.35)
            x1,y1 = NC.nucCenter(self.A-4, self.Z-2, 0.35, -0.1)
        NC.stroke(path.line(x0,y0,x1,y1), s)

class NPTrans(BaseTrans):
    """Neutron/proton decay"""
    def __init__(self, A, Z, w):
        super().__init__(A,Z,w,'n')

    def draw(self, NC):
        isProton = self.Z < 0
        A,Z = self.A, abs(self.Z)

        c = rgb(1,0.5,0) if isProton else rgb(0,0.5,1)
        alpha = color.transparency(1. - self.w)
        s = [style.linewidth.THick, c, alpha, deco.earrow([deco.filled([c])], size=NC.dscale*0.3)]

        if NC.dA[0] < 0:
            x0,y0 = NC.nucCenter(A, Z, 0.3, -0.3 if isProton else 0)
            if isProton: x1,y1 = NC.nucCenter(A-1, Z-1, -0.35, 0)
            else: x1,y1 = NC.nucCenter(A-1, Z, -0.35, 0)
        else:
            x0,y0 = NC.nucCenter(A, Z, -0.3, -0.35 if isProton else 0)
            if isProton: x1,y1 = NC.nucCenter(A-1, Z-1, 0.35, 0)
            else: x1,y1 = NC.nucCenter(A-1, Z, 0.35, 0)
        NC.stroke(path.line(x0,y0,x1,y1), s)

class ITrans(BaseTrans):
    """Isomeric Transition"""
    def __init__(self, A, Z, w):
        super().__init__(A,Z,w,'i')

    def draw(self, NC):
        x,y = NC.nucCenter(self.A, self.Z, 0.30, 0.30)
        NC.fill(path.circle(x ,y, 0.07*NC.dscale), [rgb.green, color.transparency(1. - self.w)])

class SFTrans(BaseTrans):
    """Spontaneous Fission"""
    def __init__(self, A, Z, w):
        super().__init__(A,Z,w,'f')

    def draw(self, NC):
        x,y = NC.nucCenter(self.A, self.Z)
        NC.stroke(path.circle(x, y, 0.37*NC.dscale), [style.linewidth.THICk, rgb(1,1,0), color.transparency(1. - self.w)])


class DecaySet:
    """Collection of states and transitions to draw"""
    def __init__(self):
        self.utrans = {}    # transitions to draw under states
        self.states = {}    # weighted state contents
        self.trans  = {}    # transitions to draw over states

    def addwt(d, s):
        """Add weighted item to dictionary"""
        i = s.idx()
        if i in d: d[i].w += s.w
        else: d[i] = s

    def addState(self, n): DecaySet.addwt(self.states, n)
    def addTrans(self, t, u = False): DecaySet.addwt(self.utrans if u else self.trans, t)
    def getAs(self): return [s.A for s in self.states.values()]

    def draw(self, NC):
        if not NC.Acondense: NC.condenseA(self.getAs())
        for t in self.utrans.values(): t.draw(NC)
        for s in self.states.values(): s.draw(NC)
        for t in self.trans.values():  t.draw(NC)

class NucCanvas(canvas.canvas):
    """Utilities for drawing nuclear decay chains on A:Z canvas"""
    def __init__(self):
        super().__init__()
        self.elnames =  ElementNames()
        self.dscale = 1.2 # overall drawing scale factor

        self.Acondense = {}
        self.dZ = [0, 1]
        self.dA = [-1, -3./8.]
        self.HLmin = 1e-9
        self.HLmax = 1e9


    def toZ(self, elem): return self.elnames.elNum(elem)

    def condenseA(self, As):
        """Create re-mapping for A columns to condense unused masses"""
        self.Acondense = {}
        if not As: return
        As = list(set(As))
        As.sort()
        for n in As: self.Acondense[n] = 0

        A = As[0]
        for a in As:
            self.Acondense[a] = A
            A += 1

    def nucCenter(self,A, Z, dx=0, dy=0):
        """Drawing coordinates center for nuclide"""
        A0 = self.Acondense.get(A,A)
        return (A0*self.dA[0] + Z*self.dZ[0] + dx)*self.dscale, (A*self.dA[1] + Z*self.dZ[1] + dy)*self.dscale

    def HLcolor(self, t):
        """Half-life color scheme"""
        if not t: return rgb.black
        x = log(t/self.HLmin)/log(pi*self.HLmax/self.HLmin) if t else 1
        x = min(max(0,x),1)
        return hsb(0.90*x, 1, 1)

    def HLkey(self, x0 = 0, y0 = 0):
        ts = {"1 ps": 1e-12, "10 ps": 1e-11, "100 ps": 1e-10,
              "1 ns": 1e-9, "10 ns": 1e-8, "100 ns": 1e-7,
              "1 $\\mu$s": 1e-6, "10 $\\mu$s": 1e-5, "100 $\\mu$s": 1e-4,
              "1 ms": 1e-3, "10 ms": 1e-2, "100 ms": 1e-1,
              "second": 1, "minute": 60, "hour": 3600,
              "day": 3600*24, "month": 2.63e6, "year": pi*1e7, "decade": pi*1e8, "century": pi*1e9}


        for n,(k,t) in enumerate(ts.items()):
            if t*sqrt(10.) < self.HLmin or t > self.HLmax*sqrt(10.): continue
            y = log(t/self.HLmin)/log(pi*self.HLmax/self.HLmin)
            self.text(x0*self.dscale, y0*self.dscale+7*y, k, [text.halign.boxcenter, self.HLcolor(t)])

