#! /bin/env python
"""@package NucCanvas
Draw table-of-isotopes figures"""

from AtomsDB import *
from math import *
from pyx import *
from pyx.color import rgb

class NucCanvas:
    """Utilities for drawing nuclear decay chains on A:Z canvas"""
    def __init__(self):
        self.c = canvas.canvas()
        text.set(cls=text.LatexRunner)
        text.preamble(r"\usepackage{mathtools}")
        self.nucs = {}      # isotopes (A,Z) with weights
        self.alphas = {}    # alpha decays with weights
        self.betas = {}     # beta (+positron, e.c.) decays with weights
        self.nps = {}       # neutron (proton, Z<0) decays with weights
        self.elnames =  ElementNames()
        self.dscale = 1.2       # overall drawing scale factor

        self.btri = path.path(path.moveto(0,0.2), path.lineto(0.25, -0.15),path.lineto(-0.25, -0.15),path.closepath())
        self.ptri = path.path(path.moveto(0,-0.2), path.lineto(-0.25, 0.15),path.lineto(0.25, 0.15),path.closepath())

        self.Acondense = {}
        self.dZ = (0, 1)
        self.dA = (-1, -3./8.)

    def toZ(self, elem): return self.elnames.elNum(elem)

    def condense(self):
        for A,Z in self.nucs: self.Acondense[A] = 0
        ka = list(self.Acondense.keys())
        ka.sort()
        A = ka[0]
        for k in ka:
            self.Acondense[k] = A
            A += 1

    def addwt(d, k, w):
        d.setdefault(k,0)
        d[k] += w

    def addNuc(self, A, elem, w = 1):
        self.addwt(self.nucs, (A, self.toZ(elem)), w)

    def nucCenter(self,A, Z, dx=0, dy=0):
        """Drawing coordinates center for nuclide"""
        A0 = self.Acondense.get(A,A)
        return (A0*self.dA[0] + Z*self.dZ[0] + dx)*self.dscale, (A*self.dA[1] + Z*self.dZ[1] + dy)*self.dscale

    def drawNucs(self):
        for (A,Z),w in self.nucs.items():
            x0,y0 = self.nucCenter(A, Z)
            c = color.transparency(1. - w)
            self.c.stroke(path.rect(x0-0.45*self.dscale, y0-0.45*self.dscale, 0.9*self.dscale, 0.9*self.dscale), [deformer.smoothed(radius=0.1*self.dscale), c])
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

        p0 = self.nucCenter(0,0)
        pZ = self.nucCenter(0,1)
        thz = atan2(pZ[0]-p0[0], pZ[1]-p0[1])

        if isPositron:
            x0,y0 = self.nucCenter(A, Z-0.5)
            self.c.fill(self.ptri,[rgb(0.5,0,1), color.transparency(1. - w), trafo.scale(self.dscale), trafo.rotate(180/pi * thz), trafo.translate(x0,y0)])
        else:
            x0,y0 = self.nucCenter(A, Z+0.5)
            self.c.fill(self.btri,[rgb.blue, color.transparency(1. - w), trafo.scale(self.dscale), trafo.rotate(180/pi * thz), trafo.translate(x0,y0)])

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