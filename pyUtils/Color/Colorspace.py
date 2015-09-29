from SpectrumUtils import *

###############
# Color space converisons
###############

XYZ_to_sRGB_Lin_LMat = matrix([[3.2406,-1.5372,-0.4986],
                         [-0.9689,1.8758,0.0415],
                         [0.0557,-0.2040,1.0570]])

XYZ_to_sRGB_Lin_RMat = XYZ_to_sRGB_Lin_LMat.transpose()

sRGB_Lin_to_XYZ_LMat = linalg.inv(XYZ_to_sRGB_Lin_LMat)

sRGB_Lin_to_XYZ_RMat = sRGB_Lin_to_XYZ_LMat.transpose()

CIRGB_to_XYZ_LMat = matrix([[0.454846, 0.242955, 0.0150328],
                       [0.353348, 0.67363, 0.0906372],
                       [0.157349, 0.0848316, 0.719597]]).transpose()
XYZ_to_CIRGB_LMat = linalg.inv(CIRGB_to_XYZ_LMat)

# Bradford chromatic adaptation matrix
Bradford_XYZ_to_LMS_LMat = matrix([[0.8951000, 0.2664000, -0.1614000],
                            [-0.7502000, 1.7135000, 0.0367000],
                            [0.0389000, -0.0685000, 1.0296000]])
Bradford_LMS_to_XYZ_LMat = linalg.inv(Bradford_XYZ_to_LMS_LMat)



def CIE_XYZ_to_sRGB_Lin(XYZ):
    return array( matrix(XYZ) * XYZ_to_sRGB_Lin_RMat )[0]

def sRGB_Lin_to_C(slin):
    if slin < 0:
        slin = 0
    if slin > 1:
        return 1.
    
    return slin
    
    if slin<=0.0031308:
        return 12.92*slin
    return (1.055)*slin**(1./2.4)-0.055

def sRGB_to_Lin(C):
    if C<0:
        return 0
    if C>1:
        return 1
    if C<=0.04045:
        return C/12.92
    return ((C+0.055)/(1+0.055))**2.4

def LsRGB_to_sRGB(LsRGB):
    return tuple([sRGB_Lin_to_C(LsRGB[i]) for i in range(3)])

def LsRGB_Whitepoint(illum, l0=350, l1=750):
    """Linear sRGB whitepoint for illuminant""" 
    XnYnZn = [fdot(s,illum,l0,l1) for s in cieXYZ_spectra]
    return CIE_XYZ_to_sRGB_Lin(XnYnZn)

def spectrum_sRGB(s, whitepoint):
    """sRGB color for spectrum given LsRGB whitepoint"""
    #calculate XYZ color for spectrum
    specXYZ = matrix([fdot(c,s,350,750) for c in cieXYZ_spectra])
    # calculate sRGB color for spectrum relative to whitepoint
    spec_lRGB = CIE_XYZ_to_sRGB_Lin(specXYZ)
    return LsRGB_to_sRGB([spec_lRGB[i]/whitepoint[i] for i in range(3)])
    
def CIE_XYZ_to_sRGB(XYZ,XnYnZn):
    """Convert XYZ relative to given white point to sRGB"""
    LsRGB_w = CIE_XYZ_to_sRGB_Lin(XnYnZn)
    LsRGB = CIE_XYZ_to_sRGB_Lin(XYZ)
    return tuple([sRGB_Lin_to_C(LsRGB[i]/LsRGB_w[i]) for i in range(3)])

def CIE_XYZ_Whitepoint(illum):
    """CIE X_n,Y_n,Z_n White Point under given illuminant"""
    return [fdot(s,illum,350,750) for s in cieXYZ_spectra]

def CIE_LAB_f(t):
    """L*a*b* nonlinearization function"""
    if t>(6./29.)**3:
        return t**(1./3.)
    return (29./6.)**2*t/3. + 4./29.

def CIE_LAB_df(t):
    """derivative of L*a*b* nonlinearization function"""
    if t>(6./29)**3:
        return 1./(3*t**(2./3.))
    return ((29./6.)**2)/3.

def CIE_XYZ_to_LAB(XYZ,XnYnZn):
    """Convert CIE XYZ to L*a*b*"""
    fX,fY,fZ = [CIE_LAB_f(XYZ[i]/XnYnZn[i]) for i in range(3)]
    return (116*fY,500*(fX-fY),200*(fY-fZ))

def CIE_XYZ_to_LAB_affine(XYZ,XnYnZn):
    """Local affine matrix for XYZ->LAB"""
    X,Y,Z = XYZ
    Xn,Yn,Zn = XnYnZn
    dfY = CIE_LAB_df(Y/Yn)/Yn
    return matrix([[0,                       116*dfY,  0                        ],
                [500*CIE_LAB_df(X/Xn)/Xn, -500*dfY, 0                        ],
                [0,                       200*dfY,  -200*CIE_LAB_df(Z/Zn)/Zn]])

def CIE_XYZ_to_LAB_affine_MTM(XYZ,XnYnZn,i,j):
    
    """Elements of XYZ->LAB affine matrix square for color fit calculations"""
    if i>j: # assure j>=i
        i,j = j,i
    if i==0:
        if j==0:
            dfX = CIE_LAB_df(XYZ[0]/XnYnZn[0])/XnYnZn[0]
            return 500**2*dfX*dfX
        if j==1:
            dfX = CIE_LAB_df(XYZ[0]/XnYnZn[0])/XnYnZn[0]
            dfY = CIE_LAB_df(XYZ[1]/XnYnZn[1])/XnYnZn[1]
            return -500**2*dfX*dfY
    if i==1:
        dfY = CIE_LAB_df(XYZ[1]/XnYnZn[1])/XnYnZn[1]
        if j==1:
            return (116**2+500**2+200**2)*dfY*dfY
        dfZ = CIE_LAB_df(XYZ[2]/XnYnZn[2])/XnYnZn[2]
        return -200**2*dfY*dfZ
    if i==2: # => j==2
        dfZ = CIE_LAB_df(XYZ[2]/XnYnZn[2])/XnYnZn[2]
        return 200**2*dfZ*dfZ
    return 0

def ChromAdapt_XYZ_LMat(illum1,illum2):
    """chromatic adaptation matrix for converting XYZ illum1 white point to illum2 white point"""
    W1 = Bradford_XYZ_to_LMS_LMat*matrix(CIE_XYZ_Whitepoint(illum1)).transpose()
    W2 = Bradford_XYZ_to_LMS_LMat*matrix(CIE_XYZ_Whitepoint(illum2)).transpose()
    D = matrix(diag([W2[i,0]/W1[i,0] for i in range(3)]))
    return Bradford_LMS_to_XYZ_LMat*D*Bradford_XYZ_to_LMS_LMat
