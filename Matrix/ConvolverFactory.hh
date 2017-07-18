/// \file ConvolverFactory.hh Fast convolution utilities using FFTW3
// Michael P. Mendenhall

#ifndef CONVOLVERFACTORY_HH
#define CONVOLVERFACTORY_HH

#include <complex>
#include <fftw3.h>
#include <map>
#include <vector>
#include <cassert>
using std::map;
using std::vector;

/// Base class for convolution planning
class ConvolvePlan {
public:
    /// Constructor
    ConvolvePlan(unsigned int m): M(m) { }
    /// Destructor
    virtual ~ConvolvePlan() { }

    const unsigned int M;   ///< number of elements (input)
    fftw_plan d_fwd;        ///< data real->kspace plan
    fftw_plan k_fwd;        ///< kernel real->kspace plan
    fftw_plan p_rev;        ///< convolved product kspace->real plan

    /// normalization "logical size" for given input size
    virtual unsigned int normSize() const { return 2*M; }
};

/// Real-to-complex (periodic boundary conditions) convolution workspace
class ConvolvePlanR2C: public ConvolvePlan {
public:
    /// Constructor
    ConvolvePlanR2C(unsigned int m);
    /// Destructor
    virtual ~ConvolvePlanR2C() { fftw_free(realspace); fftw_free(kspace); }

    double* realspace;      ///< array for holding real-space side of transform data
    std::complex<double>* kspace;   ///< array for holding kspace-side of transform data

    /// get FFTer for dimension m
    static ConvolvePlanR2C& get_ffter(unsigned int m);
    /// normalization "logical size" for given input size
    unsigned int normSize() const override { return M; }
protected:
    static map<unsigned int, ConvolvePlanR2C*> ffters;  ///< loaded FFTers
};

/// Real-to-real (symmetric boundary conditions) convolution workspace
class ConvolvePlanR2R: public ConvolvePlan {
public:
    /// Constructor
    ConvolvePlanR2R(unsigned int m): ConvolvePlan(m),
    realspace(fftw_alloc_real(M)), kspace(fftw_alloc_real(M)) { assert(realspace && kspace); }
    /// Destructor
    virtual ~ConvolvePlanR2R() { fftw_free(realspace); fftw_free(kspace); }
    /// multiply k-space kernel (with any appropriate shifts)
    virtual void kmul(const vector<double>& k) { for(size_t i=0; i<M; i++) kspace[i] *= k[i]; }
    /// assign k-space kernel vector
    virtual void getkKern(vector<double>& k) const { k.assign(kspace, kspace+M); }
    /// assign results to output vector
    virtual void getResult(vector<double>& v) const { v.assign(realspace, realspace+M); }


    double* realspace;      ///< array for holding real-space side of transform data
    double* kspace;         ///< array for holding kspace-side of transform data
};

/// Convolution plan for DCT-I * DCT-I -> DCT-I
/// abcd, efgh -> abcdcb * efghgf
class Convolve_DCT_I: public ConvolvePlanR2R {
public:
    /// Constructor
    Convolve_DCT_I(unsigned int m);
    /// normalization "logical size" for given input size
    unsigned int normSize() const override { return 2*(M-1); }
    /// get FFTer for dimension m
    static ConvolvePlanR2R& get_ffter(unsigned int m);
protected:
    static map<unsigned int, ConvolvePlanR2R*> ffters;  ///< loaded FFTers
};

/// Convolution plan for DCT-I * DST-I -> DST-I
/// abcd, ef ->   abcdcb * ef0FE0
class Convolve_DCT_DST_I: public ConvolvePlanR2R {
public:
    /// Constructor
    Convolve_DCT_DST_I(unsigned int m);
    /// get FFTer for dimension m
    static ConvolvePlanR2R& get_ffter(unsigned int m);
    /// normalization "logical size" for given input size
    unsigned int normSize() const override { return 2*(M-1); }
    /// multiply k-space kernel (with any appropriate shifts)
    void kmul(const vector<double>& k) override { for(size_t i=0; i<M-2; i++) kspace[i] = k[i]*kspace[i+1]; }
    /// assign k-space kernel vector
    void getkKern(vector<double>& k) const override { k.assign(kspace, kspace+M-2); }
    /// assign results to output vector
    void getResult(vector<double>& v) const override { v.assign(realspace, realspace+M-2); }
protected:
    static map<unsigned int, ConvolvePlanR2R*> ffters;  ///< loaded FFTers
};

/// Convolution plan for DCT-II * DST-II -> DST-I
/// abcd, efgh ->  abcddcba * efghHGFE
class Convolve_DCT_DST_II: public ConvolvePlanR2R {
public:
    /// Constructor
    Convolve_DCT_DST_II(unsigned int m);
    /// get FFTer for dimension m
    static ConvolvePlanR2R& get_ffter(unsigned int m);
    /// multiply k-space kernel (with any appropriate shifts)
    void kmul(const vector<double>& k) override { for(size_t i=0; i<M-1; i++) kspace[i] = k[i]*kspace[i+1]; }
    /// assign results to output vector
    void getResult(vector<double>& v) const override { v.assign(realspace, realspace+M-1); }
protected:
    static map<unsigned int, ConvolvePlanR2R*> ffters;  ///< loaded FFTers
};

//-----------------------------------
//-----------------------------------
//-----------------------------------

/// Base class for convolver (Real-to-real symmetric data/kernel), cacheing intermediate results for re-use on same-sized vectors
class ConvolverFactoryR2R {
public:
    /// Constructor
    ConvolverFactoryR2R() { }
    /// Destructor
    virtual ~ConvolverFactoryR2R() { }
    /// perform convolution
    void convolve(vector<double>& v);

protected:
    /// get appropriate plan type
    virtual ConvolvePlanR2R& getPlan(unsigned int i) const = 0;
    /// calculate real-space convolution kernel for given input size
    virtual vector<double> calcKernel(unsigned int i) const = 0;
    /// get (precalculated) k-space convolution kernel for input size
    const vector<double>& getKernel(unsigned int i);

    map<unsigned int, vector<double>> kdata;    ///< convolutions for each array size
};

/// Gaussian convolutions generator
class GaussConvolverFactory: public ConvolverFactoryR2R {
public:
    /// Constructor
    GaussConvolverFactory(double rr): r(rr) { }
    const double r;     ///< convolution radius in samples
protected:
    /// get appropriate plan type
    ConvolvePlanR2R& getPlan(unsigned int i) const override { return Convolve_DCT_I::get_ffter(i); }
    /// calculate convolution kernel for given size
    vector<double> calcKernel(unsigned int i) const override;
};

//-----------------------------------
//-----------------------------------
//-----------------------------------

/// Base class for convolver, cacheing intermediate results for re-use on same-sized vectors
class ConvolverFactoryR2C {
public:
    /// Constructor
    ConvolverFactoryR2C() { }
    /// Destructor
    virtual ~ConvolverFactoryR2C() { }
    /// perform convolution
    void convolve(vector<double>& v);

protected:
    /// calculate real-space convolution kernel for given input size
    virtual vector<double> calcKernel(unsigned int i) const = 0;
    /// get (precalculated) k-space convolution kernel for input size
    const vector<std::complex<double>>& getKernel(unsigned int i);

    map<unsigned int, vector<std::complex<double>>> kdata;    ///< convolutions for each array size
};

#endif
