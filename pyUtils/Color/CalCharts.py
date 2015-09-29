from pyx import *

def draw_colorgrid(squares,nPerRow,s=2.0,m=0.06):
    """Draw grid of sRGB-colored squares"""
    nTot = len(squares)
    nRows = ceil(float(nTot)/nPerRow)
    cOut = canvas.canvas()
    cOut.fill(path.rect(-m*s,m*s,(nPerRow+2*m)*s,(-nRows-2*m)*s))
    for i in range(nTot):
        r = i/nPerRow
        c = i%nPerRow
        cOut.fill(path.rect((c+m)*s,(-r-m)*s,(1-2*m)*s,(-1+2*m)*s),[color.rgb(squares[i][0],squares[i][1],squares[i][2])])
    return cOut

def cchecker_values(T0=6600):
    """Get ColorChecker sRGB values given illuminant temperature"""
    
    # calculation/plotting wavelength range
    l0,l1 = 380,750
    
    # blackbody illuminant
    illum = bbody_norm_spectrum(T0)
    # perfect reflector plus CC squares
    cspecs = [nd_spectrum(),]+load_specfile("ColorCheckerReflect.txt")
    # apply illuminant to each square
    cspecs = [product_spectrum([s,illum]) for s in cspecs]

    # calculate sRGB-linear color of each square from spectrum overlap with CIE X,Y,Z
    print("Calculating correct colors...")
    cieXYZ = load_specfile("CIE/CIE_1931_2deg_XYZ.txt")
    sfx = ""
    #squarecols = [CIE_XYZ_to_sRGB_Lin((fdot(cieXYZ[0],s,l0,l1), fdot(cieXYZ[1],s,l0,l1), fdot(cieXYZ[2],s,l0,l1))) for s in cspecs]
    squarecols = []
    
    # calculate camera sensor values for each square
    if 1:
        print("Calculating camera results...")
        camsens = load_specfile("Canon_5D_Spectral_Sensitivity.txt")
        camsens += [product_spectrum([camsens[2],gaussbump_spectrum(430,40,0.5)]),    # super blue
                  product_spectrum([camsens[0],longpass_spectrum(630,10)]), # super red
                  #product_spectrum([camsens[0],longpass_spectrum(590,10)]),
                  #product_spectrum([camsens[0],longpass_spectrum(530,10)]),
                  ]
        sfx = "_cam_super"
        squarecams = [ matrix([fdot(c,s,l0,l1) for c in camsens]) for s in cspecs]
        cconv = spectrum_estimator(camsens,cieXYZ)
        squarecams = [ CIE_XYZ_to_sRGB_Lin(array(c*cconv)[0]) for c in squarecams]
        
        squarecols = squarecams
    
    print("White point:",squarecols[0])
    
    # output plot
    gSquares = graph.graphxy(width=20,height=12,
                        x=graph.axis.lin(title="wavelength [nm]",min=l0-50,max=l1),
                        y=graph.axis.lin(title="spectrum",min=0,max=1.0),
                        key = graph.key.key(pos="ml"))

    scolors = []
    for i in range(len(cspecs)):
        RGB = LsRGB_to_sRGB((squarecols[i][0]/squarecols[0][0],squarecols[i][1]/squarecols[0][1],squarecols[i][2]/squarecols[0][2]))
        scolors.append(RGB)
        gSquares.plot(graph.data.points([(x,cspecs[i](x)) for x in linspace(l0,l1,400)],x=1,y=2,title="%i"%i),
                    [graph.style.line([style.linewidth.THICK]),graph.style.line([style.linewidth.THick,color.rgb(RGB[0],RGB[1],RGB[2])])])
    
    draw_colorgrid(scolors[1:],6).writetofile(os.environ["HOME"]+"/Desktop/colorchecker_recon_%i%s.pdf"%(T0,sfx), margin=0., bboxenlarge=0.)

    for i in range(len(cspecs)):
        RGB = LsRGB_to_sRGB((squarecols[i][0]/squarecols[0][0],squarecols[i][1]/squarecols[0][1],squarecols[i][2]/squarecols[0][2]))
        print(i, RGB)
        gSquares.plot(graph.data.points([(x,cspecs[i](x)) for x in linspace(l0,l1,400)],x=1,y=2,title=None), [graph.style.line([style.linewidth.THick,color.rgb(RGB[0],RGB[1],RGB[2])]),])

    gSquares.writetofile(os.environ["HOME"]+"/Desktop/colorchecker_%i%s.pdf"%(T0,sfx))
