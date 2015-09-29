#!/usr/bin/python

from SpectrumUtils import *
from SpectrumPlots import *
from Colorspace import *

def calcSensorResponse(specIn,img_illum,calSwatches,l0=350,l1=700):
    """Calculate sensor response to color chart"""
    n = len(specIn)
    ObsCols = []
    for sw in calSwatches:
        Qna = matrix(zeros((n,1)))
        for a in range(n):
            Qna[a] = fdot(product_spectrum([sw,img_illum]),specIn[a],l0,l1)
        ObsCols.append(Qna)
    return ObsCols

class SensorResponseLoader:

    def __init__(self):
        self.csquares = {}
        self.cc24_swatches = load_specfile("CC24_Reflect.txt")
        
    def load_XYZ_file(self,fname):
        flines = [l.split() for l in open(fname,"r").readlines() if len(l.split())>=5 and l[0]!='#']
        for l in flines:
            RGBval = XYZ_to_CIRGB_LMat*matrix([float(x) for x in l[2:]]).transpose()
            skey = (l[0],int(l[1]))
            self.csquares[skey] = self.csquares.setdefault(skey,[]) + [RGBval[i,0] for i in range(3)]
            
    def get_swatchnames(self):
        swkeys = self.csquares.keys()
        swkeys.sort()
        return swkeys

    def get_swatches_exp(self):
        swlist = []
        for k in self.get_swatchnames():
            if k[0]=="CC24":
                swlist.append(self.cc24_swatches[k[1]])
            else:
                raise
        return swlist
        
    def get_swatches_obs(self,swlist=None):
        if swlist is None:
            swlist = self.get_swatchnames()
        return [matrix(self.csquares[k]).transpose() for k in swlist]

    def get_CC24_grayscale(self):
        return self.get_swatches_obs([("CC24",18+i) for i in range(6)])


def CC24_blackpoint_estimate(csquares):
    """find black point for R,G,B channels"""
    targLum = [102.83637103649373, 67.64393313337078, 41.508952694742469, 23.533086320890295, 10.747454928022027, 3.9032265635123173]
    nchans = csquares[0].shape[0]
    print("\nBlack point extraction for %i channels"%nchans)
    cColors = [ [targLum[n],]+[c[i,0] for i in range(nchans)] for (n,c) in enumerate(csquares)]
    gRGB = graph.graphxy(width=12,height=12,
                 x=graph.axis.lin(title="Expected Luminance"),
                 y=graph.axis.lin(title="Channel Output"),
                 key = None)
    
    bPoint = matrix(zeros((nchans,1)))
    for i in range(nchans):
        c = (color.rgb.red,color.rgb.green,color.rgb.blue)[i%3]
        LF = LinearFitter(terms=[polyterm(0),polyterm(1)])
        LF.fit(cColors,cols=(0,1+i))
        print(LF.coeffs)
        bPoint[i] = LF.coeffs[0]
        gRGB.plot(graph.data.points(cColors,x=1,y=i+2,title=None),[graph.style.symbol(symbol.circle,symbolattrs=[c])])
        gRGB.plot(graph.data.points(LF.fitcurve(1,110),x=1,y=2,title=None),[graph.style.line([c])])

    print(bPoint)
    gRGB.writetofile(os.environ["HOME"]+"/Desktop/Chan_Lum.pdf")

    return bPoint

def writeCalFile(f,Q2RGB,bP=None):
    """Write calibration matrices to file"""
    assert Q2RGB.shape[1]%3 == 0
    nGroups = Q2RGB.shape[1]/3
    if bP is None:
        bP = matrix(zeros((3*nGroups,1)))
    assert bP.shape == (3*nGroups,1)
    
    for c in range(nGroups):
        Qi = Q2RGB[:,3*c:3*c+3]
        bPi = Qi*bP[3*c:3*c+3]
        print("Group",c)
        print(Qi)
        print(bPi)
        for i in range(3):
            for j in range(3):
                f.write("%f\t"%Qi[i,j])
            f.write("\n")
        for j in range(3):
            f.write("%f\t"%(-bPi[j,0]))
        f.write("\n")


        
def LAB_optimal_fit(SensDat,targ_illum,bPoint=None,cAdapt=None):
    """Optimize combination of input spectra to minimize LAB deviation over spectrum"""

    XnYnZn = CIE_XYZ_Whitepoint(targ_illum)
    Y0 = XnYnZn[1]
    XnYnZn = [XnYnZn[i]/Y0 for i in range(3)]
    print("White point XYZ =",XnYnZn)
    
    calSwatches = SensDat.get_swatches_exp()
    ObsCols = SensDat.get_swatches_obs()
    if bPoint is not None:
        ObsCols = [c-bPoint for c in ObsCols]
    
    n = len(ObsCols[0]) # for a,b range
    m = len(XnYnZn) # for i,j range
    M = matrix(zeros((m*n,m*n)))
    V = matrix(zeros((m*n,1)))
    
    l0,l1 = 350,700
    
    # cai layout order: c00 ... cn0 c01 ... cn1 c02 ... cn2
    XYZcols = []
    LABcols = []
    
    for (ns,sw) in enumerate(calSwatches):
        
        # Calculate response to swatch color
        Qna = ObsCols[ns]
        
        Xnl = matrix(zeros((m,1)))
        for i in range(m):
            Xnl[i] = fdot(product_spectrum([sw,targ_illum]),cieXYZ_spectra[i],l0,l1)/Y0
        XYZ = array(Xnl.transpose())[0]

        MTM = matrix(zeros((m,m)))
        for i in range(m):
            for l in range(i+1):
                MTM[i,l] = MTM[l,i] = CIE_XYZ_to_LAB_affine_MTM(XYZ,XnYnZn,i,l)

        XYZcols.append(XYZ)
        LABcols.append(CIE_XYZ_to_LAB(XYZ,XnYnZn))
        
        print("Calibration swatch %i:"%ns)
        print("\tInput:",array(Qna.transpose())[0])
        print("\tXYZ:",XYZcols[-1])
        print("\tLAB:",LABcols[-1])
    
        # add terms to mixing matrix
        for a in range(n):
            for i in range(m):
                    for l in range(m):
                        V[a+n*i] += Qna[a,0]*MTM[i,l]*Xnl[l,0]
                        for b in range(n):
                            M[a+n*i,b+n*l] += Qna[a,0]*MTM[i,l]*Qna[b,0]
                    

    print("\n\n----------------------------------------\n")
                                
    cFold = linalg.lstsq(M,V)[0]
    cOut = matrix(zeros((m,n)))
    for i in range(m):
        for a in range(n):
            cOut[i,a] = cFold[a+n*i]
    print("Q->XYZ Conversion Coefficients:")
    print(cOut)
        
    sRGB_cols = [CIE_XYZ_to_sRGB(c,XnYnZn) for c in XYZcols]
    ObsXYZ = [array((cOut*c).transpose())[0] for c in ObsCols]
    Obs_sRGB = [CIE_XYZ_to_sRGB(c,XnYnZn) for c in ObsXYZ]
    ObsLAB = [CIE_XYZ_to_LAB(c,XnYnZn) for c in ObsXYZ]

    gLAB = graph.graphxy(width=12,height=12,
                    x=graph.axis.lin(title="$a^*$",min=-60,max=60),
                    y=graph.axis.lin(title="$b^*$",min=-75,max=100),
                    key = None)
    if 0:
        gLAB.plot(graph.data.points(LABcols,x=2,y=3,title=None),[graph.style.symbol(symbol.circle),graph.style.line()])
        gLAB.plot(graph.data.points(ObsLAB,x=2,y=3,title=None),[graph.style.symbol(symbol.plus),graph.style.line()])
    else:
        for n in range(len(ObsLAB)):
            gLAB.plot(graph.data.points([LABcols[n],ObsLAB[n]],x=2,y=3,title=None),
                    [graph.style.symbol(symbol.circle),graph.style.line()])


    gLum = graph.graphxy(width=12,height=12,
                 x=graph.axis.lin(title="Expected Luminance"),
                 y=graph.axis.lin(title="Reconstructed Luminance"),
                 key = None)
    LumCompDat = [(c[1],ObsXYZ[n][1]) for (n,c) in enumerate(XYZcols)]
    gLum.plot(graph.data.points(LumCompDat,x=1,y=2,title=None),[graph.style.symbol(symbol.circle)])
    gLum.writetofile(os.environ["HOME"]+"/Desktop/Lum.pdf")

    # apply chromatic adaptation matrix and write calibration output file
    if cAdapt is not None:
        print("Using chromatic adaptation matrix:")
        print(cAdapt)
        cOut = cAdapt*cOut
    print("\nQ->CIRGB Conversion Coefficients:")
    Q2RGB = XYZ_to_CIRGB_LMat * cOut
    print(Q2RGB)
    writeCalFile(open(os.environ["HOME"]+"/Desktop/ccal.txt","w"),Q2RGB,bPoint)

    return gLAB,Obs_sRGB,sRGB_cols


if __name__=="__main__":
    
    bpath = os.environ["HOME"]+"/Desktop/MultiColor/"
        
    # sensor/filter combinations
    if 1:
        sensRGB = load_specfile("Canon_5D_Spectral_Sensitivity.txt")
        superR = product_spectrum([sensRGB[0],longpass_spectrum(630,10)])
        superOG495 = product_spectrum([sensRGB[1],longpass_spectrum(495,10)])
        superOG550 = product_spectrum([sensRGB[1],longpass_spectrum(550,10)])
        superBG495 = product_spectrum([sensRGB[2],longpass_spectrum(495,10)])
        superBG515 = product_spectrum([sensRGB[2],longpass_spectrum(515,10)])

        lp495 = [product_spectrum([longpass_spectrum(495,10),s]) for s in sensRGB]
        lp515 = [product_spectrum([longpass_spectrum(515,10),s]) for s in sensRGB]
        
        kb20 = [product_spectrum([load_specfile("B+W_KB20_Tx.txt")[0],s]) for s in sensRGB]
        bw061 = [product_spectrum([load_specfile("B+W_061_Tx.txt")[0],s]) for s in sensRGB]
        bw470 = [product_spectrum([load_specfile("B+W_470_Tx.txt")[0],s]) for s in sensRGB]
        dd491 = [product_spectrum([load_specfile("B+W_491_Tx.txt")[0],s]) for s in sensRGB]
        hpn13 = [product_spectrum([load_specfile("Heliopan_13_Tx.txt")[0],s]) for s in sensRGB]
        
        wr47b = [product_spectrum([load_specfile("Wratten_47b_Tx.txt")[0],s]) for s in sensRGB]
        superB = wr47b[-1]
        wr58 = [product_spectrum([load_specfile("Wratten_58_Tx.txt")[0],s]) for s in sensRGB]
        wr56 = [product_spectrum([load_specfile("Wratten_56_Tx.txt")[0],s]) for s in sensRGB]
        
        mag40 = [product_spectrum([load_specfile("Wratten_CC40M_Tx.txt")[0],s]) for s in sensRGB]
        mag50 = [product_spectrum([load_specfile("Wratten_CC50M_Tx.txt")[0],s]) for s in sensRGB]
        mag70 = [product_spectrum([pow_spectrum(load_specfile("Wratten_CC50M_Tx.txt")[0],7./5.),s]) for s in sensRGB]
    
    ###### define calculation ######
    comboname = "RGB+47B"
    sensors = sensRGB+[superB, mag70[-1]]
    img_illum = CIE_D_Illum(75)
    whitepoint = LsRGB_Whitepoint(img_illum)
    dest_illum = CIE_D_Illum(50)
    cAdapt = ChromAdapt_XYZ_LMat(img_illum,dest_illum)

    plot_spectra_colors(sensors, whitepoint = whitepoint).writetofile(bpath+"/sensors_%s.pdf"%comboname) # spectral sensitivity with illuminant
    exit(0)

    #ObsCols = calcSensorResponse(sensors,img_illum,calSwatches)
    #bPoint = None
    ObsCols = SensorResponseLoader()
    ObsCols.load_XYZ_file(os.environ["HOME"]+"/Desktop/Z/IMG_2713_charts_XYZ.txt")
    ObsCols.load_XYZ_file(os.environ["HOME"]+"/Desktop/Z/IMG_2715_charts_XYZ.txt")
    bPoint = CC24_blackpoint_estimate(ObsCols.get_CC24_grayscale())
    
    gLAB,Obs_sRGB,Exp_sRGB = LAB_optimal_fit(ObsCols,img_illum,bPoint,cAdapt)
    gLAB.writetofile(bpath+"/cc24_lab_%s.pdf"%comboname)
    draw_colorgrid(Obs_sRGB,6).writetofile(bpath+"/CC24_%s.pdf"%comboname, margin=0., bboxenlarge=0.)
    