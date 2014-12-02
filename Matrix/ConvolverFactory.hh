#ifndef CONVOLVERFACTORY_HH
#define CONVOLVERFACTORY_HH

#include <fftw3.h>
#include <map>
#include <vector>
#include <complex.h>

using std::vector;
using std::map;
using std::complex;

/// calculate nearest "FFTW-friendly" size within dmax of i
int fftw_best_nearest_size(int i, int dmax);

/// Stores fftw data for FFT'ing
class Convolver_FFT {
public:
    /// constructor
    Convolver_FFT(unsigned int m);
    /// destructor
    ~Convolver_FFT() { delete[] realspace; delete[] kspace; }
    
    const unsigned int M;       ///< number of elements
    fftw_plan forwardplan;      ///< FFTW data for forward Fourier Transforms of this size
    fftw_plan reverseplan;      ///< FFTW data for inverse Fourier Transforms of this size
    double* realspace;          ///< array for holding real-space side of transform data
    complex<double>* kspace;    ///< array for holding kspace-side of transform data
    
    /// get FFTer for dimension m
    static Convolver_FFT& get_ffter(unsigned int m);
    
protected:
    
    static map<unsigned int,Convolver_FFT*> ffters;  ///< loaded FFTers
};

/// Base class for convolver, cacheing intermediate results for re-use on same-sized vectors
class ConvolverFactory {
public:
    /// Constructor
    ConvolverFactory() { }
    
    /// Destructor
    virtual ~ConvolverFactory() { }
    
    /// perform convolution
    void convolve(vector<double>& v);
    
protected:
    
    /// calculate convolution kernel for given size
    virtual vector<double> calcKernel(unsigned int i) const = 0;
    /// generate k-space convolution kernel
    virtual const vector< complex<double> >& getKernel(unsigned int i);
    
    map<unsigned int, vector< complex<double> > > kdata;        ///< convolutions for each array size
};

/// Gaussian convolutions generator
class GaussConvolverFactory: public ConvolverFactory {
public:
    /// Constructor
    GaussConvolverFactory(double rr): r(rr) { }
    
    const double r;     ///< convolution radius in samples
protected:
    
    /// calculate convolution kernel for given size
    virtual vector<double> calcKernel(unsigned int i) const;
};


#endif
