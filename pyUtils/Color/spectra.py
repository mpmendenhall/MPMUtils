#!/usr/bin/python

from SpectrumUtils import *
from SpectrumPlots import *
import os

if __name__=="__main__":
    
    bpath = os.environ["HOME"]+"/Desktop/"
    
    sensRGB = load_specfile("Canon_5D_Spectral_Sensitivity.txt")
    wr47b = [product_spectrum([load_specfile("Wratten_47b_Tx.txt")[0],s]) for s in sensRGB]
    k5113 = [product_spectrum([load_specfile("Kopp_5113_Tx.txt")[0],s]) for s in sensRGB]
    mag70 = [product_spectrum([pow_spectrum(load_specfile("Wratten_CC50M_Tx.txt")[0],7./5.),s]) for s in sensRGB]
    kb20 = [product_spectrum([load_specfile("B+W_KB20_Tx.txt")[0],s]) for s in sensRGB]
    superB = wr47b[-1]
    ugab = load_specfile("UGAB_Emission.txt")[0]
    ugab.name = "UG emission"
    ug_tx = rescale_spectrum(pow_spectrum(load_specfile("UGAB_Li_BigBatch_Tx.txt")[0],7))
    ug_tx.name = "UG Transmission"
    
    flash = bbody_norm_spectrum(5400)
    flash.name = "5400K illuminant"
    whitepoint = LsRGB_Whitepoint(flash)
    
    sensors = [sensRGB[2], wr47b[2], k5113[2]]
    sensors = [product_spectrum([s,flash]) for s in sensors]
    for s in sensors:
        s.plotcolor = spectrum_sRGB(s,whitepoint)
    sensors = [rescale_spectrum(s) for s in sensors]
    sensors.append(rescale_spectrum(ugab))
    sensors.append(ug_tx)
    sensors[0].name = "Canon 5D blue"
    sensors[1].name = "Wratten 47b filtered"
    sensors[2].name = "Kopp 5113 filtered"
    
    for s in sensors[:4]:
        print "Transmission for",s.name,fdot(s,ug_tx)/spectrum_integral(s)
    
    gSpectra = graph.graphxy(width=10,height=8,
                            x=graph.axis.lin(title="wavelength [nm]",min=380,max=620),
                            y=graph.axis.lin(title="spectrum [arb. units]",min=0,max=1.1),
                            key = graph.key.key(pos="mr"))
    
    plot_spectra_colors(sensors+[flash,], gSpectra=gSpectra, whitepoint = whitepoint)
    gSpectra.writetofile(bpath+"/sensors.pdf") # spectral sensitivity with illuminant
    