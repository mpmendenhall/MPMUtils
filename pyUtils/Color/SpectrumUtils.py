#!/sw/bin/python2.7

from math import *
from numpy import *
from pyx import *
from pyx.graph.style import symbol
from scipy.integrate import quad
from bisect import bisect
from LinFitter import *

###############
# utility functions
###############

def fdot(f1,f2,x0=350,x1=750,fw=(lambda x: 1.0)):
	"""Inner product between two functions over specififed range"""
	return quad((lambda x:f1(x)*f2(x)*fw(x)),x0,x1,epsabs=1.e-6, epsrel=1.e-4)[0]

class cubterpolator:
	"""Quick-and-dirty cubic interpolator"""
	def __init__(self,xydat):
		xydat.sort()
		self.xpts = [x[0] for x in xydat]
		self.ypts = [x[1] for x in xydat]
	
	def __call__(self,x):
		b = bisect(self.xpts,x)
		if b<1 or b>=len(self.xpts):
			return 0
		l = (x-self.xpts[b-1])/(self.xpts[b]-self.xpts[b-1])
		if b<2 or b>=len(self.xpts)-1:
			return self.ypts[b-1]*(1-l)+self.ypts[b]*l
		
		A = -0.5
		p0 = self.ypts[b-2]
		p1 = self.ypts[b-1]
		p2 = self.ypts[b]
		p3 = self.ypts[b+1]
		return A*p0*(1-l)*(1-l)*l + p1*(1-l)*(1-l*((2+A)*l-1)) - p2*l*(A*(1-l)*(1-l)+l*(2*l-3)) + A*p3*(1-l)*l*l


###############
# Spectrum generators
###############

class nd_spectrum:
	"""'Neutral density' (constant value) spectrum"""
	def __init__(self,h=1.0):
		self.h = h
	def __call__(self,x):
		return self.h

class gaussbump_spectrum:
	"""Gaussian 'bump' spectrum with center, width, and height"""
	def __init__(self,l0,w,h=1.0):
		self.l0 = l0
		self.w = w
		self.h = h
	def __call__(self,l):
		return self.h*exp(-(l-self.l0)**2/(2*self.w**2))

class longpass_spectrum:
	"""Long pass with 50% point, transition width"""
	def __init__(self,l0,w):
		self.l0 = l0
		self.w = w
	def __call__(self,l):
		return 0.5*(1+erf((l-self.l0)/self.w))

class bbody_norm_spectrum:
	"""Blackbody spectrum, normalized to height 1 at (estimated) peak"""
	def __init__(self,T):
		self.T = T
		# normalization at estimated peak
		self.nrm = 1.0/self.spec_unnorm(2.897835e6/self.T)
	def spec_unnorm(self,l):
		# un-normalized BB spectrum
		return 1/((exp(1.438775e7/(l*self.T))-1)*l**5)
	def __call__(self,l):
		return self.spec_unnorm(l)*self.nrm

class product_spectrum:
	"""Product of a set of spectra"""
	def __init__(self,ss):
		self.ss = ss
	def __call__(self,l):
		return prod([s(l) for s in self.ss])

class pow_spectrum:
	def __init__(self,s,p):
		self.s = s
		self.p = p
	def __call__(self,l):
		return self.s(l)**self.p

def load_specfile(fname,fDir=None):
	"""Load text columns file; first column for wavelength, spectra in each other column"""
	if fDir is None:
		fDir = "../SpectraDat"
	fdat = [[float(x) for x in l.split()] for l in open(fDir+"/"+fname,"r").readlines() if len(l.split())>=2 and l[0] in "0123456789+-"]
	ncols = len(fdat[0])
	if not ncols:
		print "Spectrum data not found in %s/%s"%(fDir,fname)
		raise
	return [cubterpolator([(l[0],l[i]) for l in fdat if len(l)==ncols]) for i in range(ncols)[1:]]

class CIE_D_Illum:
	def __init__(self,T):
		"""CIE 'D' Series standard daylight illuminants, normalized to 1 at 560nm"""
		#D50,55,65,75 series, correction for improved Planck constant
		if T<300:
			T = 100*T*1.4388/1.438
		
		self.T = T
		if T<7000:
			self.x = 0.244063 + 0.09911e3/T + 2.9678e6/T**2 - 4.6070e9/T**3
		else:
			self.x = 0.237040 + 0.24748e3/T + 1.9018e6/T**2 - 2.0064e9/T**3
		self.y = -3.000*self.x**2 + 2.870*self.x - 0.275
		self.SD = load_specfile("CIE_D_Illuminants.txt")
		self.M = 0.0241 + 0.2562*self.x - 0.7341*self.y
		self.M1 = (-1.3515 - 1.7703*self.x + 5.9114*self.y)/self.M
		self.M2 = (0.0300 - 31.4424*self.x + 30.0717*self.y)/self.M
		self.nrm = 1.0
		self.nrm = 1.0/self(560)
	
	def __call__(self,l):
		return (self.SD[0](l) + self.M1*self.SD[1](l) + self.M2*self.SD[2](l))*self.nrm


###############
# important spectra
###############

cieXYZ_spectra = load_specfile("CIE_1931_2deg_XYZ_1nm.txt")
cieLMS_spectra = load_specfile("CIE2006_LMS.txt")
