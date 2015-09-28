#!/sw/bin/python2.7

from SpectrumUtils import *
from Colorspace import *


def spectrum_estimator(inSpecs,outSpecs):
	"""model output spectra as linear combination of inputs"""
	n = len(inSpecs)
	M = matrix(zeros((n,n)))
	l0,l1 = 400,700
	for i in range(n):
		for j in range(i+1):
			M[i,j] = fdot(inSpecs[i],inSpecs[j],l0,l1)/(l1-l0)
			M[j,i] += M[i,j]
	Mi = linalg.inv(M)
	print "Spectrum Correlations Matrix"
	print M
	print "Correlation Inverse"
	print Mi

	m = len(outSpecs)
	V = matrix(zeros((n,m)))
	for i in range(n):
		for j in range(m):
			V[i,j] = fdot(inSpecs[i],outSpecs[j],l0,l1)/(l1-l0)
	C = Mi*V
	print "Output correlation"
	print V
	print "Output conversion"
	print C

	# calculate errors
	print "Reconstruction errors:"
	for r in range(m):
		err = 0
		for i in range(n):
			for j in range(n):
				if i==j:
					err += C[i,r]*C[j,r]*M[i,j]*0.5
				else:
					err += C[i,r]*C[j,r]*M[i,j]
			err -= C[i,r]*V[i,r]
		rr = fdot(outSpecs[r],outSpecs[r],l0,l1)/(l1-l0)
		err += rr
		err /= rr
		print r,err

	return C

def spectrum_recon(inSpecs,outSpecs):
	l0,l1 = 350,750
	n,m = len(inSpecs),len(outSpecs)
	C = spectrum_estimator(inSpecs,outSpecs)
	gIn = graph.graphxy(width=15,height=10,
					x=graph.axis.lin(title="wavelength [nm]",min=l0,max=l1),
					y=graph.axis.lin(title="spectral sensitivity",min=0,max=2))
	for i in range(n):
		gIn.plot(graph.data.points([(x,inSpecs[i](x)) for x in unifrange(l0,l1,200)],x=1,y=2), [graph.style.line(lineattrs=[style.linestyle.dashed,]),])
	for j in range(m):
		gIn.plot(graph.data.points([(x,outSpecs[j](x)) for x in unifrange(l0,l1,200)],x=1,y=2), [graph.style.line([color.rgb.blue]),])
		gIn.plot(graph.data.points([(x,sum([inSpecs[i](x)*C[i,j] for i in range(n)])) for x in unifrange(l0,l1,200)],x=1,y=2), [graph.style.line([style.linestyle.dotted,color.rgb.red]),])

	gIn.writetofile("/Users/michael/Desktop/inSpecs.pdf")

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


def plot_spectra_colors(illum,specs,l0=380,l1=750,ymx=1,ymn=0):
	"""Plot spectra in color"""

	# white point
	XnYnZn = [fdot(s,illum,l0,l1) for s in cieXYZ_spectra]
	lRGB0 = CIE_XYZ_to_sRGB_Lin(XnYnZn)
	
	# calculate each XYZ response
	specXYZ = [ matrix([fdot(c,s,l0,l1) for c in cieXYZ_spectra]) for s in specs]
	# calculate sRGB for each patch
	spec_lRGB = [ CIE_XYZ_to_sRGB_Lin(c) for c in specXYZ]
	spec_RGB = [ LsRGB_to_sRGB([c[i]/lRGB0[i] for i in range(3)]) for c in spec_lRGB]

	# plot results
	gSquares = graph.graphxy(width=20,height=12,
						x=graph.axis.lin(title="wavelength [nm]",min=l0-50,max=l1),
						y=graph.axis.lin(title="spectrum",min=ymn,max=ymx),
						key = graph.key.key(pos="ml"))
	for i in range(len(specs)):
		rgbCol = color.rgb(spec_RGB[i][0],spec_RGB[i][1],spec_RGB[i][2])
		gSquares.plot(graph.data.points([(x,specs[i](x)) for x in unifrange(l0,l1,400)],x=1,y=2,title="%i"%i),
				    [graph.style.line([style.linewidth.THICK]),graph.style.line([style.linewidth.THick,rgbCol])])
		gSquares.plot(graph.data.points([(x,specs[i](x)) for x in unifrange(l0,l1,400)],x=1,y=2,title=None),
				    [graph.style.line([style.linewidth.THick,rgbCol]),])

	return gSquares
					
def cchecker_values(T0=6600):
	
	# calculation/plotting wavelength range
	l0,l1 = 380,750
	
	# blackbody illuminant
	illum = bbody_norm_spectrum(T0)
	# perfect reflector plus CC squares
	cspecs = [nd_spectrum(),]+load_specfile("ColorCheckerReflect.txt")
	# apply illuminant to each square
	cspecs = [product_spectrum([s,illum]) for s in cspecs]

	# calculate sRGB-linear color of each square from spectrum overlap with CIE X,Y,Z
	print "Calculating correct colors..."
	cieXYZ = load_specfile("CIE/CIE_1931_2deg_XYZ.txt")
	sfx = ""
	#squarecols = [CIE_XYZ_to_sRGB_Lin((fdot(cieXYZ[0],s,l0,l1), fdot(cieXYZ[1],s,l0,l1), fdot(cieXYZ[2],s,l0,l1))) for s in cspecs]
	squarecols = []
	
	# calculate camera sensor values for each square
	if 1:
		print "Calculating camera results..."
		camsens = load_specfile("Canon_5D_Spectral_Sensitivity.txt")
		camsens += [product_spectrum([camsens[2],gaussbump_spectrum(430,40,0.5)]),	# super blue
				  product_spectrum([camsens[0],longpass_spectrum(630,10)]), # super red
				  #product_spectrum([camsens[0],longpass_spectrum(590,10)]),
				  #product_spectrum([camsens[0],longpass_spectrum(530,10)]),
				  ]
		sfx = "_cam_super"
		squarecams = [ matrix([fdot(c,s,l0,l1) for c in camsens]) for s in cspecs]
		cconv = spectrum_estimator(camsens,cieXYZ)
		squarecams = [ CIE_XYZ_to_sRGB_Lin(array(c*cconv)[0]) for c in squarecams]
		
		squarecols = squarecams
	
	print "White point:",squarecols[0]
	
	# output plot
	gSquares = graph.graphxy(width=20,height=12,
						x=graph.axis.lin(title="wavelength [nm]",min=l0-50,max=l1),
						y=graph.axis.lin(title="spectrum",min=0,max=1.0),
						key = graph.key.key(pos="ml"))

	scolors = []
	for i in range(len(cspecs)):
		RGB = LsRGB_to_sRGB((squarecols[i][0]/squarecols[0][0],squarecols[i][1]/squarecols[0][1],squarecols[i][2]/squarecols[0][2]))
		scolors.append(RGB)
		gSquares.plot(graph.data.points([(x,cspecs[i](x)) for x in unifrange(l0,l1,400)],x=1,y=2,title="%i"%i),
				    [graph.style.line([style.linewidth.THICK]),graph.style.line([style.linewidth.THick,color.rgb(RGB[0],RGB[1],RGB[2])])])
	
	draw_colorgrid(scolors[1:],6).writetofile("/Users/michael/Desktop/colorchecker_recon_%i%s.pdf"%(T0,sfx), margin=0., bboxenlarge=0.)

	for i in range(len(cspecs)):
		RGB = LsRGB_to_sRGB((squarecols[i][0]/squarecols[0][0],squarecols[i][1]/squarecols[0][1],squarecols[i][2]/squarecols[0][2]))
		print i, RGB
		gSquares.plot(graph.data.points([(x,cspecs[i](x)) for x in unifrange(l0,l1,400)],x=1,y=2,title=None), [graph.style.line([style.linewidth.THick,color.rgb(RGB[0],RGB[1],RGB[2])]),])

	gSquares.writetofile("/Users/michael/Desktop/colorchecker_%i%s.pdf"%(T0,sfx))


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
	print "\nBlack point extraction for %i channels"%nchans
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
		print LF.coeffs
		bPoint[i] = LF.coeffs[0]
		gRGB.plot(graph.data.points(cColors,x=1,y=i+2,title=None),[graph.style.symbol(symbol.circle,symbolattrs=[c])])
		gRGB.plot(graph.data.points(LF.fitcurve(1,110),x=1,y=2,title=None),[graph.style.line([c])])

	print bPoint
	gRGB.writetofile("/Users/michael/Desktop/Chan_Lum.pdf")

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
		print "Group",c
		print Qi
		print bPi
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
	print "White point XYZ =",XnYnZn
	
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
		
		print "Calibration swatch %i:"%ns
		print "\tInput:",array(Qna.transpose())[0]
		print "\tXYZ:",XYZcols[-1]
		print "\tLAB:",LABcols[-1]
	
		# add terms to mixing matrix
		for a in range(n):
			for i in range(m):
					for l in range(m):
						V[a+n*i] += Qna[a,0]*MTM[i,l]*Xnl[l,0]
						for b in range(n):
							M[a+n*i,b+n*l] += Qna[a,0]*MTM[i,l]*Qna[b,0]
					

	print "\n\n----------------------------------------\n"
								
	cFold = linalg.lstsq(M,V)[0]
	cOut = matrix(zeros((m,n)))
	for i in range(m):
		for a in range(n):
			cOut[i,a] = cFold[a+n*i]
	print "Q->XYZ Conversion Coefficients:"
	print cOut
		
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
	gLum.writetofile("/Users/michael/Desktop/Lum.pdf")

	# apply chromatic adaptation matrix and write calibration output file
	if cAdapt is not None:
		print "Using chromatic adaptation matrix:"
		print cAdapt
		cOut = cAdapt*cOut
	print "\nQ->CIRGB Conversion Coefficients:"
	Q2RGB = XYZ_to_CIRGB_LMat * cOut
	print Q2RGB
	writeCalFile(open("/Users/michael/Desktop/ccal.txt","w"),Q2RGB,bPoint)

	return gLAB,Obs_sRGB,sRGB_cols



if __name__=="__main__":
	
	bpath = "/Users/michael/Desktop/MultiColor/"
		
	# sensor/filter combinations
	if 0:
		sensRGB = load_specfile("Canon_5D_Spectral_Sensitivity.txt")
		superR = product_spectrum([sensRGB[0],longpass_spectrum(630,10)])
		superB = product_spectrum([sensRGB[2],load_specfile("Wratten/Wratten_Curve_47b.txt")[0]])
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
		
		wr58 = [product_spectrum([load_specfile("Wratten_58_Tx.txt")[0],s]) for s in sensRGB]
		wr56 = [product_spectrum([load_specfile("Wratten_56_Tx.txt")[0],s]) for s in sensRGB]
		
		mag40 = [product_spectrum([load_specfile("Wratten_CC40M_Tx.txt")[0],s]) for s in sensRGB]
		mag50 = [product_spectrum([load_specfile("Wratten_CC50M_Tx.txt")[0],s]) for s in sensRGB]
		mag70 = [product_spectrum([pow_spectrum(load_specfile("Wratten_CC50M_Tx.txt")[0],7./5.),s]) for s in sensRGB]
	
	###### define calculation ######
	comboname = "RGB+CC70M"
	#sensors = sensRGB
	img_illum = CIE_D_Illum(75)
	dest_illum = CIE_D_Illum(50)
	cAdapt = ChromAdapt_XYZ_LMat(img_illum,dest_illum)

	#plot_spectra_colors(img_illum,sensors).writetofile(bpath+"/sensors_%s.pdf"%comboname) # spectral sensitivity with illuminant
	
	#ObsCols = calcSensorResponse(sensors,img_illum,calSwatches)
	#bPoint = None
	ObsCols = SensorResponseLoader()
	ObsCols.load_XYZ_file("/Users/michael/Desktop/Z/IMG_2713_charts_XYZ.txt")
	ObsCols.load_XYZ_file("/Users/michael/Desktop/Z/IMG_2715_charts_XYZ.txt")
	bPoint = CC24_blackpoint_estimate(ObsCols.get_CC24_grayscale())
	
	gLAB,Obs_sRGB,Exp_sRGB = LAB_optimal_fit(ObsCols,img_illum,bPoint,cAdapt)
	gLAB.writetofile(bpath+"/cc24_lab_%s.pdf"%comboname)
	draw_colorgrid(Obs_sRGB,6).writetofile(bpath+"/CC24_%s.pdf"%comboname, margin=0., bboxenlarge=0.)




