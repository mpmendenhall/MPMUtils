#!/usr/bin/python

import numpy
from numpy import *
import numpy.linalg as linalg
import cmath

def product(l):
    p = 1
    for i in l:
        p *= i
    return p
    
def basisv(n,m=-1):
    return tuple([int(l==m) for l in range(n)])
        
# polynomial, represented by a dictionary with tuples of exponents as the keys
# mapped to the coefficient of the term
class polynomial:
    """Class for manipulating polynomials in N variables"""
    
    def __init__(self,N,coeffs={}):
        self.N = N
        self.C0 = (0,)*self.N
        self.coeffs = coeffs.copy()
        self.varnames = ( "x","y","z","t", "w","v", "u", "a", "b", "c" )

    def add_monomial(self,C,v):
        """Add monomial term vC"""
        if C in self.coeffs:
            self.coeffs[C] += v
        else:
            self.coeffs[C] = v

    def __add__(self,other):
        """Add polynomials"""
        p = self.copy()
        if isinstance(other,polynomial):
            assert other.N == self.N
            for k,v in other.coeffs.items():
                p.add_monomial(k,v)
        else:
            p.add_monomial(p.C0,other)
        return p
    
    def __mul__(self,other):
        """Multiply polynomial or scalar type"""
        p = polynomial(self.N)
        if isinstance(other,polynomial):
            assert other.N == self.N
            for k1 in self.coeffs.keys():
                for k2 in other.coeffs.keys():
                    k3 = tuple([k1[i]+k2[i] for i in range(len(k1))])
                    p.coeffs[k3] = p.coeffs.get(k3,0) + self.coeffs[k1]*other.coeffs[k2]
        else:
            for k1 in self.coeffs.keys():
                p.coeffs[k1] = self.coeffs[k1]*other 
        return p
        
    def __neg__(self):
        """Negate polynomial"""
        p = polynomial(self.N)
        for k in self.coeffs.keys():
            p.coeffs[k] = -self.coeffs[k]
        return p
        
    def __sub__(self,other):
        """Subtract polynomials"""
        return self + (-other)

    def __pow__(self,n):
        """Raise polynomial to an integer power"""
        assert n==int(n)
        assert n >= 0
        if n==0:
            return polynomial(self.N,{self.C0: 1})
        if n==1:
            return self.copy()
        else:
            p = self.copy()
            return p*p**(n-1)


    def __call__(self,varvals):
        """Evaluate at given variable values"""
        s = 0
        for k in self.coeffs.keys():
            ss = self.coeffs[k]
            for i in range(len(k)):
                ss *= varvals[i]**k[i]
            s += ss
        return s
    
    
    def getTerms(self,unityCoeffs=True):
        """Get each term, optionally with coefficients set to unity"""
        ks = self.coeffs.keys()
        ks.sort()
        Ts = []
        for k in ks:
            P = polynomial(self.N)
            P.coeffs[k] = 1 if unityCoeffs else self.coeffs[k]
            Ts.append(P)
        return Ts
    
    def evalTerms(self,varvals):
        """Evaluate term-by-term; useful for linear fits"""
        return [ product([varvals[n]**v for (n,v) in enumerate(k)])*self.coeffs[k] for k in self.coeffs.keys() ]
    
    def rescale(self,scalefactors):
        """Scale each variable by given scale factor x->s*x"""
        for k in self.coeffs.keys():
            self.coeffs[k] *= product([ scalefactors[n]**p for (n,p) in enumerate(k) ])

    def evalOne(self,n,xval):
        """Evaluate out one variable at given value; return scalar if is scalar"""
        p = polynomial(self.N-1)
        for k in self.coeffs.keys():
            nkey = k[:n]+k[n+1:]
            p.coeffs[nkey] = p.coeffs.get(nkey,0) + self.coeffs[k]*(xval**k[n])
        if p.N == 0:
            return p.coeffs.get((),0)
        return p
        
    def derivative(self,n):
        """Derivative over variable n"""
        p = polynomial(self.N)
        for k in self.coeffs.keys():
            if k[n] == 0:
                continue
            c = self.coeffs[k]*k[n]
            kn = list(k); kn[n] -= 1; kn = tuple(kn);
            p.coeffs[kn] = p.coeffs.get(kn,0) + c
        return p
    
    def indefIntegral(self,n):
        """Indefinite integral in variable n"""
        p = polynomial(self.N)
        for k in self.coeffs.keys():
            c = self.coeffs[k]/(1.0+k[n])
            kn = list(k); kn[n] += 1; kn = tuple(kn);
            p.coeffs[kn] = p.coeffs.get(kn,0) + c
        return p
        
    def integral(self,n,a,b):
        """Definite integral integrating out variable n"""
        p = self.indefIntegral(n)
        return p.evalOne(n,b) - p.evalOne(n,a)
        
    def average(self,n,a,b):
        """Average over range in variable b"""
        return self.integral(n,a,b)*(1.0/(b-a))
                
    def copy(self):
        """Copy into another polynomial"""
        p = polynomial(self.N,self.coeffs)
        p.varnames = self.varnames
        return p
        
    def quadratic_derivs_matrix(self):
        """Matrix and vector to solve for minima of quadratic, LHS*x0 = RHS. Note, LHS is symmetric, hence diagonalizable with orthogonal basis."""
        lhs = matrix([ [ self.derivative(dv).coeffs[basisv(self.N,v)] for v in range(self.N) ] for dv in range(self.N) ])
        rhs = matrix([ self.derivative(dv).coeffs[self.C0] for dv in range(self.N) ]).transpose()
        return (lhs, rhs)
    
    def quadratic_extrema(self):
        """Extremum location for multi-quadratic polynomial"""
        lhs,rhs = self.quadratic_derivs_matrix()
        return array(linalg.solve(lhs,rhs).transpose())[0]
        
    def max_error_bounds(self):
        P = self.copy()
        P.coeffs[(0,)*self.nvars()] = 0
        for i in range(self.nvars()):
            P.coeffs[basisv(self.nvars(),i)] = 0
        m = linalg.pinv(P.quadratic_derivs_matrix()[0])
        return [ m[r,r]/sqrt(2*P( [ -x for x in m.transpose()[r].tolist()[0] ] )) for r in range(self.nvars()) ]
    
    def __repr__(self):
        return "<polynomial %s>"%str(self.coeffs)
        
        s = ""
        if len(self.coeffs) < 10:
            ks = self.coeffs.keys()
            ks.sort()
            if not ks:
                s = "0"
            for k in ks:
                if self.coeffs[k] == 0:
                    continue
                s += " "+str(self.coeffs[k]);
                for i in range(self.N):
                    if k[i] == 0:
                        continue
                    s += "%s"%self.varnames[i]
                    if k[i] > 1:
                        s += "%i"%k[i]
        else:
            s = "%i vars, %i terms"%(self.N,len(self.coeffs))
        return "<polynomial %s>"%s
        
    def tostring(self):
        """Display as a string"""
        s = ""
        ks = self.coeffs.keys()
        if not ks:
            return "0"
        ks.sort()
        for k in ks:
            if self.coeffs[k] == 0:
                continue
            s += "\t%+g"%self.coeffs[k];
            for i in range(len(k)):
                if k[i] == 0:
                    continue
                s += " %s"%self.varnames[i]
                if k[i] > 1:
                    s += "^%i"%k[i]
                    
        return s[1:]

    def set_coeff_to_first(self,i):
        """Re-order the i^th coefficient to first"""
        if i==0:
            return
        self.varnames = list(self.varnames)
        self.varnames = tuple([self.varnames[i],] + self.varnames[:i]+self.varnames[i+1:])
        newcoeffs = {}
        for c in self.coeffs:
            cnew = [c[i],]+list(c[:i])+list(c[i+1:])
            newcoeffs[tuple(cnew)] = self.coeffs[c]
        self.coeffs = newcoeffs

def monomial(C,v=1.):
    """Convenience constructor for a monomial term"""
    coeff = tuple(C)
    return polynomial(len(coeff),{coeff:v})


def poly_change_of_variable(p0,xnew):
    """Polynomial change-of-variable x_i -> P_i(x)"""
    p = p0.copy()
    p.coeffs = {}

    for t in p0.coeffs:
        q = polynomial(p0.N)
        q += p0.coeffs[t]
        for (n,m) in enumerate(t):
            if m:
                q *= xnew[n]**m
        p += q
    
    return p

def recenter_poly(p,c):
    """Origin-shifting change-of-variable x_i -> x_i + c_i"""
    xnew = [ polynomial(p.N, {p.C0 : c[n], basisv(p.N,n) : 1.0}) for n in range(p.N) ]
    return poly_change_of_variable(p,xnew)

def map_poly_to_unit_cell(p,ll,ur):
    """Scale and translate polynomial to map region ll to ur onto [-1/2,1/2]^N"""
    xnew = [ polynomial(p.N, {p.C0 : 0.5*(ll[n]+ur[n]), basisv(p.N,n) : ur[n]-ll[n]}) for n in range(p.N) ]
    return poly_change_of_variable(p,xnew)

def Fourier_transform_poly(p,i,k):
    """Fourier transform out the i^th variable of polynomial, \int_{-1/2}^{1/2} P(x) e^{-2*pi*i*k*x} dx"""
    
    if k==0:
        return p.integral(i,-0.5,0.5)

    p0 = p.copy()
    p0.set_coeff_to_first(i)
    pF = polynomial(p.N-1)
    pF.varnames = p0.varnames[1:]

    while p0.coeffs:
        clist = p0.coeffs.keys()
        clist.sort()
        C = clist[-1]
        m = C[0]
                    
        v = p0.coeffs.pop(C)
        
        if m==0:
            if k != int(k):
                pF.add_monomial(C[1:],v*sin(pi*k)/(pi*k))
            continue
        
        d = -1./(2j*pi*k)
        if m%2 or k != int(k):
            c0 = 2**(-m)*(cmath.exp(-1j*pi*k) - (-1)**m*cmath.exp(1j*pi*k))
            pF.add_monomial(C[1:],v*d*c0)

        C = list(C);
        C[0] -= 1
        p0.add_monomial(tuple(C),-m*v*d)

    if not pF.N:
        return pF.coeffs.get((),0)
    return pF

def Polynomial_Fourier_coeff(p0,kvec):
    """Fourier coefficient integrated over all variables"""
    assert len(kvec) == p0.N
    p = p0.copy()
    for k in kvec:
        p = Fourier_transform_poly(p,0,k)
    if isinstance(p,polynomial):
        p = p.coeffs.get((),0)
    return p


def lowTriangTerms(nVars, order):
    """Polynomial of sepcified order and number of variables, e.g. (2,2) -> x^2+xy+y^2+x+y+1"""
    assert order>=0
    if order==0:
        P=polynomial(nVars)
        P.coeffs[P.C0] = 0
        return P
    P = lowTriangTerms(nVars,order-1)
    Q = P.copy()
    for n in range(nVars):
        Q += P*monomial(basisv(nVars,n),0)
    return Q



