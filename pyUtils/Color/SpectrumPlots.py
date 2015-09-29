from pyx import *
from pyx.graph.style import symbol
from numpy import linspace
from Colorspace import *

def spectrum_recon(inSpecs,outSpecs):
    """Plot linear combination spectra estimates"""
    l0,l1 = 350,750
    n,m = len(inSpecs),len(outSpecs)
    C = spectrum_estimator(inSpecs,outSpecs)
    gIn = graph.graphxy(width=15,height=10,
                    x=graph.axis.lin(title="wavelength [nm]",min=l0,max=l1),
                    y=graph.axis.lin(title="spectral sensitivity",min=0,max=2))
    for i in range(n):
        gIn.plot(graph.data.points([(x,inSpecs[i](x)) for x in linspace(l0,l1,200)],x=1,y=2), [graph.style.line(lineattrs=[style.linestyle.dashed,]),])
    for j in range(m):
        gIn.plot(graph.data.points([(x,outSpecs[j](x)) for x in linspace(l0,l1,200)],x=1,y=2), [graph.style.line([color.rgb.blue]),])
        gIn.plot(graph.data.points([(x,sum([inSpecs[i](x)*C[i,j] for i in range(n)])) for x in linspace(l0,l1,200)],x=1,y=2), [graph.style.line([style.linestyle.dotted,color.rgb.red]),])

    gIn.writetofile(os.environ["HOME"]+"/Desktop/inSpecs.pdf")
    
def plot_spectra_colors(specs, l0=350, l1=750, gSpectra = None, whitepoint = (105.6,105.7,105.7)):
    """Plot spectra in color"""
    
    if not gSpectra:
        gSpectra = graph.graphxy(width=20,height=12,
                            x=graph.axis.lin(title="wavelength [nm]",min=l0,max=l1),
                            y=graph.axis.lin(title="spectrum",min=0,max=1),
                            key = graph.key.key(pos="mr"))
        
    for (i,s) in enumerate(specs):
        rgbCol = None
        if hasattr(s,"plotcolor"):
            if type(s.plotcolor) == type(color.rgb.red):
                rgbCol = s.plotcolor
            else:
                rgbCol = color.rgb(s.plotcolor[0],s.plotcolor[1],s.plotcolor[2])
        if not rgbCol:
            spec_RGB = spectrum_sRGB(s,whitepoint)
            rgbCol = color.rgb(spec_RGB[0],spec_RGB[1],spec_RGB[2])
        
        title = "%i"%i
        if hasattr(s,"name"):
            title = s.name

        gSpectra.plot(graph.data.points([(x,s(x)) for x in linspace(l0,l1,400)],x=1,y=2,title=title),
                    [graph.style.line([style.linewidth.THIck]),graph.style.line([style.linewidth.THick,rgbCol])])
        # re-plot line core for clearer overlaps
        gSpectra.plot(graph.data.points([(x,s(x)) for x in linspace(l0,l1,400)],x=1,y=2,title=None),
                    [graph.style.line([style.linewidth.THick,rgbCol]),])

    return gSpectra
