#!/usr/bin/python

from SpectrumUtils import *
from SpectrumPlots import *
import os

if __name__=="__main__":
    
    bpath = os.environ["HOME"]+"/Desktop/MultiColor/"
    
    sensRGB = load_specfile("Canon_5D_Spectral_Sensitivity.txt")
    wr47b = [product_spectrum([load_specfile("Wratten_47b_Tx.txt")[0],s]) for s in sensRGB]
    mag70 = [product_spectrum([pow_spectrum(load_specfile("Wratten_CC50M_Tx.txt")[0],7./5.),s]) for s in sensRGB]
    superB = wr47b[-1]
    
    flash = bbody_norm_spectrum(5400)
    flash.name = "5400K illuminant"
    whitepoint = LsRGB_Whitepoint(flash)
    
    sensors = [sensRGB[2], mag70[2], wr47b[2]]
    sensors = [rescale_spectrum(product_spectrum([s,flash])) for s in sensors]
    sensors[0].name = "Canon 5D blue"
    sensors[1].name = "CC70M filtered"
    sensors[2].name = "Wratten 47b filtered"
    
    gSpectra = graph.graphxy(width=10,height=8,
                            x=graph.axis.lin(title="wavelength [nm]",min=350,max=750),
                            y=graph.axis.lin(title="spectrum [arb. units]",min=0,max=1.1),
                            key = graph.key.key(pos="mr"))
    
    plot_spectra_colors(sensors+[flash,], gSpectra=gSpectra, whitepoint = whitepoint)
    gSpectra.writetofile(bpath+"/sensors.pdf") # spectral sensitivity with illuminant
    