#! /bin/env python
"""@package NucCanvas
Draw table-of-isotopes figures"""

from AtomsDB import *
from math import *
from pyx import *
from pyx.color import rgb, hsb

text.set(cls=text.LatexRunner)
text.preamble(r"\usepackage{mathtools}")

class NucCanvas:
    """Utilities for drawing nuclear decay chains on A:Z canvas"""
    def __init__(self):
        self.c = canvas.canvas()
        self.nucs = {}      # isotopes (A,Z,[L]) with weights
        self.HL = {}        # half-lives for isotopes
        self.alphas = {}    # alpha decays with weights
        self.betas = {}     # beta (+positron, e.c.) decays with weights
        self.nps = {}       # neutron (proton, Z<0) decays with weights
        self.its = {}       # IT transitions
        self.SFs = {}       # spontaneous fission markers
        self.elnames =  ElementNames()
        self.dscale = 1.2       # overall drawing scale factor

        self.btri = path.path(path.moveto(0,0.2), path.lineto(0.25, -0.15),path.lineto(-0.25, -0.15),path.closepath())
        self.ptri = path.path(path.moveto(0,-0.2), path.lineto(-0.25, 0.15),path.lineto(0.25, 0.15),path.closepath())

        self.Acondense = {}
        self.dZ = (0, 1)
        self.dA = (-1, -3./8.)

    def toZ(self, elem): return self.elnames.elNum(elem)

    def condense(self):
        for n in self.nucs: self.Acondense[n[0]] = 0
        ka = list(self.Acondense.keys())
        ka.sort()
        A = ka[0]
        for k in ka:
            self.Acondense[k] = A
            A += 1

    def addwt(d, k, w):
        d.setdefault(k,0)
        d[k] += w

    def addNuc(self, A, elem, w = 1, L = None):
        self.addwt(self.nucs, (A, self.toZ(elem), L), w)

    def nucCenter(self,A, Z, dx=0, dy=0):
        """Drawing coordinates center for nuclide"""
        A0 = self.Acondense.get(A,A)
        return (A0*self.dA[0] + Z*self.dZ[0] + dx)*self.dscale, (A*self.dA[1] + Z*self.dZ[1] + dy)*self.dscale

    def HLcolor(t):
        if not t: return rgb.black
        x = log(t/1e-12)/log(pi*1e9/1e-12) if t else 1
        x = min(max(0,x),1)
        return hsb(0.90*x, 1, 1)

    def HLkey(outname = "hlKey"):
        c = canvas.canvas()
        text.set(cls=text.LatexRunner)
        ts = {"1 ps": 1e-12, "10 ps": 1e-11, "100 ps": 1e-10,
              "1 ns": 1e-9, "10 ns": 1e-8, "100 ns": 1e-7,
              "1 $\\mu$s": 1e-6, "10 $\\mu$s": 1e-5, "100 $\\mu$s": 1e-4,
              "1 ms": 1e-3, "10 ms": 1e-2, "100 ms": 1e-1,
              "second": 1, "minute": 60, "hour": 3600,
              "day": 3600*24, "month": 2.63e6, "year": pi*1e7, "decade": pi*1e8, "century": pi*1e9}

        for n,(k,t) in enumerate(ts.items()):
            y = log(t/1e-12)/log(pi*1e9/1e-12)
            c.text(0, 7*y, k, [text.halign.boxcenter, NucCanvas.HLcolor(t)])
        if outname is not None: c.writePDFfile(outname)
        else: return c

    def drawNucs(self):

        for (A,Z),w in self.SFs.items(): self.drawSF(A,Z,w)
        for (A,Z),w in self.its.items(): self.drawIT(A,Z,w)

        for n,w in self.nucs.items():
            A,Z = n[0], n[1]
            x0,y0 = self.nucCenter(A, Z)
            c = color.transparency(1. - w)
            d = 0.9
            if len(n) >= 3 and n[2]: d -= 0.05*n[2]
            fc = self.__class__.HLcolor(self.HL.get(n,None))

            self.c.stroke(path.rect(x0-0.5*d*self.dscale, y0-0.5*d*self.dscale, d*self.dscale, d*self.dscale), [deformer.smoothed(radius=0.1*self.dscale), fc, c])



            y0 -= 0.1*self.dscale
            c = color.transparency(min(0.9, 1. - w))
            self.c.text(x0, y0, r"$^{%i}_{%i}$%s"%(A,Z,self.elnames.elSym(Z)), [text.halign.boxcenter, c])

        for (A,Z),w in self.betas.items(): self.drawBeta(A,Z,max(0.1,w))
        for (A,Z),w in self.alphas.items(): self.drawAlpha(A,Z,max(0.1,w))
        for (A,Z),w in self.nps.items(): self.drawNeutron(A,Z,max(0.1,w))

    def drawBeta(self, A, Z, w=1):
        """Draw marker for beta transition"""
        isPositron = Z < 0
        Z = abs(Z)

        if isPositron:
            x0,y0 = self.nucCenter(A, Z-0.5)
            c = rgb(1,0,1)
        else:
            x0,y0 = self.nucCenter(A, Z+0.5)
            c = rgb.blue

        p0 = self.nucCenter(A,Z)
        pZ = self.nucCenter(A,Z+(-1 if isPositron else 1))
        thz = atan2(pZ[0]-p0[0], pZ[1]-p0[1])
        self.c.fill(self.btri,
                    [c, color.transparency(1. - w), trafo.scale(self.dscale), trafo.rotate(180/pi * thz), trafo.translate(x0,y0)])

    def drawAlpha(self,A,Z,w=1):
        """Draw marker for alpha transition"""
        x0,y0 = self.nucCenter(A,   Z,   0.3, -0.3)
        x1,y1 = self.nucCenter(A-4, Z-2, -0.35, 0)
        self.c.stroke(path.line(x0,y0,x1,y1),[style.linewidth.THick, rgb.red, color.transparency(1. - w), deco.earrow([deco.filled([rgb.red])], size=self.dscale*0.3)])

    def drawNeutron(self,A,Z,w=1):
        isProton = Z < 0
        Z = abs(Z)

        x0,y0 = self.nucCenter(A,   Z,   0.3, -0.3 if isProton else 0)
        c = rgb(1,0.5,0) if isProton else rgb(0,0.5,1)
        s = [style.linewidth.THick, c, color.transparency(1. - w), deco.earrow([deco.filled([c])], size=self.dscale*0.3)]
        if isProton: x1,y1 = self.nucCenter(A-1, Z-1, -0.35, 0)
        else: x1,y1 = self.nucCenter(A-1, Z, -0.35, 0)
        self.c.stroke(path.line(x0,y0,x1,y1), s)

    def drawIT(self,A,Z,w=1):
        """Draw marker for IT transition"""
        x,y = self.nucCenter(A, Z, 0.30, 0.30)
        self.c.fill(path.circle(x ,y, 0.07*self.dscale), [rgb.green, color.transparency(1. - w)])

    def drawSF(self,A,Z,w=1):
        """Draw marker for spontaneous fission"""
        x,y = self.nucCenter(A, Z)
        self.c.stroke(path.circle(x ,y, 0.37*self.dscale), [style.linewidth.THICk, rgb(1,1,0), color.transparency(1. - w)])
