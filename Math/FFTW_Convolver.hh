/// \file FFTW_Convolver.h Fast convolution utilities using FFTW3
// Michael P. Mendenhall, 2019

#ifndef FFTW_CONVOLVER_H
#define FFTW_CONVOLVER_H
#ifndef WITHOUT_FFTW

#include <complex>
#include <fftw3.h>
#include <map>
using std::map;
#include <vector>
using std::vector;
#include <mutex>

/// convenience definition for complex datatype
typedef std::complex<double> cplx_t;

/// allocator wrapper for FFTW allocate functions
template <class T>
struct fftw_allocator {
    typedef T value_type;

    /// default constuctor
    fftw_allocator() = default;

    /// constructor from alternate type
    template <class U>
    constexpr fftw_allocator(const fftw_allocator<U>&) noexcept {}

    /// allocation, with bounds checks
    T* allocate(std::size_t n) {
        if(n > std::numeric_limits<std::size_t>::max() / sizeof(T)) throw std::bad_alloc();
        if(auto p = _allocate(n)) return p;
        throw std::bad_alloc();
    }

    /// unchecked underlying allocator call
    T* _allocate(std::size_t n) { return (T*)std::malloc(n*sizeof(T)); }

    /// deallocation
    void deallocate(T* p, std::size_t) noexcept { std::free(p); }
};
template <class T, class U>
bool operator==(const fftw_allocator<T>&, const fftw_allocator<U>&) { return true; }
template <class T, class U>
bool operator!=(const fftw_allocator<T>&, const fftw_allocator<U>&) { return false; }

template<>
inline double* fftw_allocator<double>::_allocate(size_t m) { return fftw_alloc_real(m); }
template<>
inline cplx_t* fftw_allocator<cplx_t>::_allocate(size_t m) { return (cplx_t*)fftw_alloc_complex(m); }
template<>
inline void fftw_allocator<double>::deallocate(value_type* p, size_t) noexcept { fftw_free(p); }
template<>
inline void fftw_allocator<cplx_t>::deallocate(value_type* p, size_t) noexcept { fftw_free(p); }

/// FFTW-allocated real data
typedef vector<double, fftw_allocator<double>> fftwvec_r;
/// FFTW-allocated complex data
typedef vector<cplx_t, fftw_allocator<cplx_t>> fftwvec_c;

//-////////////////////////////////////////////////////
// Collection of FFT plans for a fixed-size convolution

/// Base class for convolution planning (thread-safe)
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


//-//////////////////////////////////////
// Workspaces for fixed size convolutions

/// Real-to-complex (periodic boundary conditions) convolution workspace
class ConvolvePlanR2C: public ConvolvePlan {
public:
    fftwvec_r realspace;    ///< array for holding real-space side of transform data
    fftwvec_c kspace;       ///< array for holding kspace-side of transform data

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
    /// multiply k-space kernel (with any appropriate shifts)
    virtual void kmul(const double* k, double* ks = nullptr) { if(!ks) ks = kspace.data(); for(size_t i=0; i<M; i++) ks[i] *= k[i]; }
    /// assign results to output vector
    virtual void getResult(vector<double>& v, const double* rs = nullptr) const { if(!rs) rs = realspace.data(); v.assign(rs, rs+M); }

    fftwvec_r realspace;    ///< array for holding real-space side of transform data
    fftwvec_r kspace;       ///< array for holding kspace-side of transform data
protected:
    /// Constructor
    ConvolvePlanR2R(size_t m): ConvolvePlan(m), realspace(M), kspace(M) { }
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
    void kmul(const double* k, double* ks = nullptr) override { if(!ks) ks = kspace.data(); for(size_t i=0; i<M-2; i++) ks[i] = k[i]*ks[i+1]; }
    /// assign results to output vector
    void getResult(vector<double>& v, const double* rs = nullptr) const override { if(!rs) rs = realspace.data(); v.assign(rs, rs+M-2); }
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
    void kmul(const double* k, double* ks = nullptr) override { if(!ks) ks = kspace.data(); for(size_t i=0; i<M-1; i++) ks[i] = k[i]*ks[i+1]; }
    /// assign results to output vector
    void getResult(vector<double>& v, const double* rs = nullptr) const override { if(!rs) rs = realspace.data(); v.assign(rs, rs+M-1); }
protected:
    /// Constructor
    Convolve_DCT_DST_II(size_t m);
};


//-//////////////////////////////////////////
// "Factories" for multiple convolution sizes

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
    const vector<double>& getKernel(size_t i);

    map<size_t, vector<double>> kdata;  ///< convolutions for each array size
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
    /// perform convolution
    void convolve(vector<double>& v);

protected:
    /// calculate real-space convolution kernel for given input size
    virtual void calcKernel(size_t i, double* v) const = 0;
    /// get (precalculated) k-space convolution kernel for input size
    const vector<cplx_t>& getKernel(size_t i);

    map<size_t, vector<cplx_t>> kdata;  ///< convolutions for each array size
    std::mutex kernLock;                ///< multithreading lock for constructing kernels
};

#endif
#endif
