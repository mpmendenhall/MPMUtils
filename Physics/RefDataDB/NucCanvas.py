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
        text.set(mode="latex")
        text.preamble(r"\usepackage{mathtools}")
        self.nucs = {}      # isotopes (A,Z) with weights
        self.alphas = {}    # alpha decays with weights
        self.betas = {}     # beta decays with weights
        self.elnames =  ElementNames()
        self.dscale = 1.2       # overall drawing scale factor

        self.btri = path.path(path.moveto(0,0.2), path.lineto(0.25, -0.15),path.lineto(-0.25, -0.15),path.closepath())

        p0 = self.nucCenter(0,0)
        pZ = self.nucCenter(0,1)
        self.dz = (pZ[0]-p0[0], pZ[1]-p0[1])

    def toZ(self, elem): return self.elnames.elNum(elem)

    def addwt(d, k, w):
        d.setdefault(k,0)
        d[k] += w

    def addNuc(self, A, elem, w = 1):
        self.addwt(self.nucs, (A, self.toZ(elem)), w)

    def nucCenter(self,A,Z):
        """Drawing coordinates center for nuclide"""
        return -0.5*A*self.dscale, (Z-3./8.*A)*self.dscale # good for alpha/beta chain

    def drawNucs(self):
        for (A,Z),w in self.nucs.items():
            x0,y0 = self.nucCenter(A, Z)
            c = color.transparency(1. - w)
            self.c.stroke(path.rect(x0-0.45*self.dscale, y0-0.45*self.dscale, 0.9*self.dscale, 0.9*self.dscale), [c])
            y0 -= 0.1*self.dscale
            self.c.text(x0, y0, r"$^{%i}_{%i}$%s"%(A,Z,self.elnames.elSym(Z)), [text.halign.boxcenter, c])

        for (A,Z),w in self.betas.items(): self.drawBeta(A,Z,w)
        for (A,Z),w in self.alphas.items(): self.drawAlpha(A,Z,w)

    def drawBeta(self,A,Z,w=1):
        """Draw marker for beta transition"""
        x0,y0 = self.nucCenter(A,Z+0.5)
        self.c.fill(self.btri,[rgb.blue, color.transparency(1. - w), trafo.scale(self.dscale), trafo.rotate(180/pi * atan2(self.dz[0],self.dz[1])), trafo.translate(x0,y0)])

    def drawAlpha(self,A,Z,w=1):
        """Draw marker for alpha transition"""
        x0,y0 = self.nucCenter(A-0.5, Z-0.5)
        x1,y1 = self.nucCenter(A-1.4, Z-1.5)
        self.c.stroke(path.line(x0,y0,x1,y1),[style.linewidth.THick, rgb.red, color.transparency(1. - w), deco.earrow([deco.filled([rgb.red])], size=self.dscale*0.3)])
