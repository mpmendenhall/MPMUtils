#!/sw/bin/python

from pyx import *
from math import *

class ArrowPlotter:

    def __init__(self,g):
        self.g = g                                # graph to plot to
        self.gsize = (g.width,g.height)            # size of graph canvas
        self.gaxes = (g.axes['x'].axis, g.axes['y'].axis)
        self.axisrange = [ (a.min,a.max) for a in self.gaxes ] # axis ranges of graph
        self.lscale = [ self.gsize[n]*1.0/(r[1]-r[0]) for (n,r) in enumerate(self.axisrange) ] # length scaling, axis units to cm
        
    # adat = [ [x0,y0,dx,dy], ... ]
    def plot_arrowdat(self,adat,arrowSty=graph.style.arrow(lineattrs=[style.linewidth.Thick]),endsize=0.3,offset=False):
        
        if offset:
            adat = [ [p[0]+0.5*p[2],p[1]+0.5*p[3],p[2],p[3]] for p in adat]
        datscaled = [ [p[0], p[1], p[2]*self.lscale[0], p[3]*self.lscale[1]] for p in adat]
        topolar = [ [p[0], p[1], sqrt(p[2]**2+p[3]**2), 180.*atan2(p[3],p[2])/pi ] for p in datscaled]
        
        # scale properly with graph size
        arrowSty.linelength = unit.v_cm
        arrowSty.arrowsize = endsize*unit.v_cm
        
        self.g.plot(graph.data.points(topolar,x=1,y=2,size=3,angle=4), [arrowSty])
        

if __name__=="__main__":


    g = graph.graphxy(width=14,height=14,
                    x=graph.axis.lin(min=0,max=10),
                    y=graph.axis.lin(min=-6,max=4))

    AP = ArrowPlotter(g)
    
    adat = []
    for i in range(11):
        for j in range(11):
            x0 = i
            y0 = (8.*j)/10-6
            dx = j/10.;
            dy = -i/10.;
            adat.append([x0,y0,dx,dy])
        
    asty = graph.style.arrow(lineattrs=[style.linewidth.Thick])
    AP.plot_arrowdat(adat,asty,offset=True)
    
    g.writetofile("ArrowTest.pdf")
