/// \file FFTW_Convolver.h Fast convolution utilities using FFTW3
// Michael P. Mendenhall, 2018

#ifndef FFTW_CONVOLVER_H
#define FFTW_CONVOLVER_H
#ifndef WITHOUT_FFTW

#include <complex>
#include <fftw3.h>
#include <map>
#include <vector>
#include <mutex>
#include <cassert>
using std::map;
using std::vector;

/// convenience definition for complex datatype
typedef std::complex<double> cplx_t;

/// Pool of "recyclable" allocated data arrays
template<typename T>
class ArrayPool {
public:
    /// get allocated array
    static T* get(size_t m) {
        std::unique_lock<std::mutex> lk(A().allocLock);
        auto& v = A().arrays[m];
        if(!v.size()) {
            v.push_back(alloc(m));
            A().array_sz[v.back()] = m;
        }
        auto a = v.back();
        v.pop_back();
        return a;
    }
    /// return array after use
    static void release(T* a) {
        std::unique_lock<std::mutex> lk(A().allocLock);
        auto sz = A().array_sz.at(a);
        A().arrays[sz].push_back(a);
    }

protected:
    /// Constructor
    ArrayPool() { }
    /// allocate new array
    static T* alloc(size_t m);

    ///< singleton instance for this type
    static ArrayPool<T>& A();

    /// allocated arrays by size
    map<size_t, vector<T*>> arrays;
    /// array sizes
    map<T*, size_t> array_sz;
    /// lock on allocation calls
    std::mutex allocLock;
};

template<>
double* ArrayPool<double>::alloc(size_t m);
template<>
cplx_t* ArrayPool<cplx_t>::alloc(size_t m);


/// Base class for convolution planning
class ConvolvePlan {
public:
    /// Constructor
    ConvolvePlan(size_t m): M(m) { }
    /// Destructor
    virtual ~ConvolvePlan() { }

    const size_t M;         ///< number of elements (input)
    fftw_plan d_fwd;        ///< data real->kspace plan
    fftw_plan k_fwd;        ///< kernel real->kspace plan
    fftw_plan p_rev;        ///< convolved product kspace->real plan

    /// normalization "logical size" for given input size
    virtual size_t normSize() const { return 2*M; }

protected:
    static std::mutex fftLock;  ///< multithreading lock for constructing plans
};

/// Real-to-complex (periodic boundary conditions) convolution workspace
class ConvolvePlanR2C: public ConvolvePlan {
public:
    /// Destructor
    virtual ~ConvolvePlanR2C() { ArrayPool<double>::release(realspace); ArrayPool<cplx_t>::release(kspace); }

    double* realspace;  ///< array for holding real-space side of transform data
    cplx_t* kspace;     ///< array for holding kspace-side of transform data

    /// get FFTer for dimension m
    static ConvolvePlanR2C& get_ffter(size_t m);
    /// normalization "logical size" for given input size
    size_t normSize() const override { return M; }
protected:
    /// Constructor
    ConvolvePlanR2C(size_t m);
};

/// Real-to-real (symmetric boundary conditions) convolution workspace
class ConvolvePlanR2R: public ConvolvePlan {
public:
    /// Destructor
    virtual ~ConvolvePlanR2R() { ArrayPool<double>::release(realspace); ArrayPool<double>::release(kspace); }
    /// multiply k-space kernel (with any appropriate shifts)
    virtual void kmul(const double* k, double* ks = nullptr) { if(!ks) ks = kspace; for(size_t i=0; i<M; i++) ks[i] *= k[i]; }
    /// assign k-space kernel vector
    virtual void getkKern(vector<double>& k) const { k.assign(kspace, kspace+M); }
    /// assign results to output vector
    virtual void getResult(vector<double>& v, const double* rs = nullptr) const { if(!rs) rs = realspace; v.assign(rs, rs+M); }

    double* realspace;      ///< array for holding real-space side of transform data
    double* kspace;         ///< array for holding kspace-side of transform data
protected:
    /// Constructor
    ConvolvePlanR2R(size_t m): ConvolvePlan(m),
    realspace(ArrayPool<double>::get(M)), kspace(ArrayPool<double>::get(M)) { }
};

/// Convolution plan for DCT-I * DCT-I -> DCT-I
/// abcd, efgh -> abcdcb * efghgf
class Convolve_DCT_I: public ConvolvePlanR2R {
public:
    /// normalization "logical size" for given input size
    size_t normSize() const override { return 2*(M-1); }
    /// get FFTer for dimension m
    static ConvolvePlanR2R& get_ffter(size_t m);
protected:
    /// Constructor
    Convolve_DCT_I(size_t m);
};

/// Convolution plan for DCT-I * DST-I -> DST-I
/// abcd, ef ->   abcdcb * ef0FE0
class Convolve_DCT_DST_I: public ConvolvePlanR2R {
public:
    /// get FFTer for dimension m
    static ConvolvePlanR2R& get_ffter(size_t m);
    /// normalization "logical size" for given input size
    size_t normSize() const override { return 2*(M-1); }
    /// multiply k-space kernel (with any appropriate shifts)
    void kmul(const double* k, double* ks = nullptr) override { if(!ks) ks = kspace; for(size_t i=0; i<M-2; i++) ks[i] = k[i]*ks[i+1]; }
    /// assign k-space kernel vector
    void getkKern(vector<double>& k) const override { k.assign(kspace, kspace+M-2); }
    /// assign results to output vector
    void getResult(vector<double>& v, const double* rs = nullptr) const override { if(!rs) rs = realspace; v.assign(rs, rs+M-2); }
protected:
    /// Constructor
    Convolve_DCT_DST_I(size_t m);
};

/// Convolution plan for DCT-II * DST-II -> DST-I
/// abcd, efgh ->  abcddcba * efghHGFE
class Convolve_DCT_DST_II: public ConvolvePlanR2R {
public:
    /// get FFTer for dimension m
    static ConvolvePlanR2R& get_ffter(size_t m);
    /// multiply k-space kernel (with any appropriate shifts)
    void kmul(const double* k, double* ks = nullptr) override { if(!ks) ks = kspace; for(size_t i=0; i<M-1; i++) ks[i] = k[i]*ks[i+1]; }
    /// assign results to output vector
    void getResult(vector<double>& v, const double* rs = nullptr) const override { if(!rs) rs = realspace; v.assign(rs, rs+M-1); }
protected:
    /// Constructor
    Convolve_DCT_DST_II(size_t m);
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
    virtual ConvolvePlanR2R& getPlan(size_t i) const = 0;
    /// calculate real-space convolution kernel for given input size
    virtual void calcKernel(size_t i, double* v) const = 0;
    /// get (precalculated) k-space convolution kernel for input size
    const double* getKernel(size_t i);

    map<size_t, double*> kdata; ///< convolutions for each array size
    std::mutex kernLock;        ///< multithreading lock for constructing kernels
};

/// Gaussian convolutions generator
class GaussConvolverFactory: public ConvolverFactoryR2R {
public:
    /// Constructor
    GaussConvolverFactory(double rr): r(rr) { }
    const double r;     ///< convolution radius in samples
protected:
    /// get appropriate plan type
    ConvolvePlanR2R& getPlan(size_t i) const override { return Convolve_DCT_I::get_ffter(i); }
    /// calculate convolution kernel for given size
    void calcKernel(size_t i, double* v) const override;
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
    virtual ~ConvolverFactoryR2C() { for(auto& kv: kdata) ArrayPool<cplx_t>::release(kv.second); }
    /// perform convolution
    void convolve(vector<double>& v);

protected:
    /// calculate real-space convolution kernel for given input size
    virtual void calcKernel(size_t i, double* v) const = 0;
    /// get (precalculated) k-space convolution kernel for input size
    const cplx_t* getKernel(size_t i);

    map<size_t, cplx_t*> kdata;    ///< convolutions for each array size
    std::mutex kernLock;           ///< multithreading lock for constructing kernels
};

#endif
#endif
