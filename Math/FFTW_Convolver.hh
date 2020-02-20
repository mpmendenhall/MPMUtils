/// \file FFTW_Convolver.hh Fast convolution utilities using FFTW3
// Michael P. Mendenhall, LLNL 2020

#ifndef FFTW_CONVOLVER_H
#define FFTW_CONVOLVER_H

#include "fftwx.hh"
#include <map>
#include <vector>
#include <mutex>
using std::map;
using std::vector;

/// Base class for convolution planning
template<typename T = double>
class ConvolvePlan: public fftwx<T> {
public:
    typedef typename fftwx<T>::plan_t plan_t;

    /// Constructor
    ConvolvePlan(unsigned int m): M(m) { }
    /// Destructor
    ~ConvolvePlan() { }

    const unsigned int M;   ///< number of elements (input)
    plan_t d_fwd;           ///< data real->kspace plan
    plan_t k_fwd;           ///< kernel real->kspace plan
    plan_t p_rev;           ///< convolved product kspace->real plan

    /// normalization "logical size" for given input size
    virtual unsigned int normSize() const { return 2*M; }

    static std::mutex& plannerLock();   ///< lock on using FFTW to generate plans
};

template<>
std::mutex& ConvolvePlan<double>::plannerLock();
template<>
std::mutex& ConvolvePlan<float>::plannerLock();
template<>
std::mutex& ConvolvePlan<long double>::plannerLock();

inline int planner_flags(int M) {
    if(M <= 0) return FFTW_EXHAUSTIVE | FFTW_DESTROY_INPUT;
    return FFTW_PATIENT | FFTW_DESTROY_INPUT;
}

/// Real-to-complex (periodic boundary conditions) convolution workspace
template<typename T = double>
class ConvolvePlanR2C: public ConvolvePlan<T> {
public:
    typedef typename fftwx<T>::fcplx_t fcplx_t;

    /// Constructor
    ConvolvePlanR2C(unsigned int m): ConvolvePlan<T>(m), realspace(m), kspace(m/2+1) {
        this->d_fwd = this->plan_dft_r2c_1d(m, realspace.data(), (fcplx_t*)kspace.data(), planner_flags(m));
        this->k_fwd = this->plan_dft_r2c_1d(m, realspace.data(), (fcplx_t*)kspace.data(), planner_flags(m));
        this->p_rev = this->plan_dft_c2r_1d(m, (fcplx_t*)kspace.data(), realspace.data(), planner_flags(m));
    }

    fftwvec_r<T> realspace; ///< real-space side of transform data
    fftwvec_c<T> kspace;    ///< kspace-side of transform data

    /// get FFTer for dimension m
    static ConvolvePlanR2C& get_ffter(unsigned int m) {
        static map<unsigned int, ConvolvePlanR2C<T>*> ffters;
        std::lock_guard<std::mutex> lk(ConvolvePlan<T>::plannerLock());
        auto it = ffters.find(m);
        if(it != ffters.end()) return *(it->second);
        return *(ffters[m] = new ConvolvePlanR2C<T>(m));
    }
    /// normalization "logical size" for given input size
    unsigned int normSize() const override { return this->M; }
};

/// Real-to-real (symmetric boundary conditions) convolution workspace
template<typename T = double>
class ConvolvePlanR2R: public ConvolvePlan<T> {
public:
    /// Constructor
    ConvolvePlanR2R(unsigned int m): ConvolvePlan<T>(m), realspace(m), kspace(m) { }

    /// multiply k-space kernel (with any appropriate shifts)
    virtual void kmul(const vector<T>& k) { for(size_t i=0; i<this->M; i++) kspace[i] *= k[i]; }
    /// assign k-space kernel vector
    virtual void getkKern(vector<T>& k) const { k.assign(kspace.begin(), kspace.end()); }
    /// assign results to output vector
    virtual void getResult(vector<T>& v) const { v.assign(realspace.begin(), realspace.end()); }

    fftwvec_r<T> realspace; ///< real-space side of transform data
    fftwvec_r<T> kspace;    ///< kspace-side of transform data
};


//-----------------------------------
//-----------------------------------
//-----------------------------------


/// Convolution plan for DCT-I * DCT-I -> DCT-I
/// abcd, efgh -> abcdcb * efghgf
template<typename T = double>
class Convolve_DCT_I: public ConvolvePlanR2R<T> {
public:
    /// Constructor
    Convolve_DCT_I(unsigned int m): ConvolvePlanR2R<T>(m) {
        this->d_fwd = this->plan_r2r_1d(m, this->realspace.data(), this->kspace.data(), FFTW_REDFT00, planner_flags(m));
        this->k_fwd = this->d_fwd;
        this->p_rev = this->plan_r2r_1d(m, this->kspace.data(), this->realspace.data(), FFTW_REDFT00, planner_flags(m));
    }

    /// get FFTer for dimension m
    static ConvolvePlanR2R<T>& get_ffter(unsigned int m) {
        static map<unsigned int,ConvolvePlanR2R<T>*> ffters;
        std::lock_guard<std::mutex> lk(ConvolvePlan<T>::plannerLock());
        auto it = ffters.find(m);
        if(it != ffters.end()) return *(it->second);
        return *(ffters[m] = new Convolve_DCT_I<T>(m));
    }

    /// normalization "logical size" for given input size
    unsigned int normSize() const override { return 2*(this->M-1); }
};

/// Convolution plan for DCT-I * DST-I -> DST-I
/// abcd, ef ->   abcdcb * ef0FE0
template<typename T = double>
class Convolve_DCT_DST_I: public ConvolvePlanR2R<T> {
public:
    /// Constructor
    Convolve_DCT_DST_I(unsigned int m): ConvolvePlanR2R<T>(m) {
        this->d_fwd = this->plan_r2r_1d(m,   this->realspace.data(), this->kspace.data(), FFTW_REDFT00, planner_flags(m));
        this->k_fwd = this->plan_r2r_1d(m-2, this->realspace.data(), this->kspace.data(), FFTW_RODFT00, planner_flags(m));
        this->p_rev = this->plan_r2r_1d(m-2, this->kspace.data(), this->realspace.data(), FFTW_RODFT00, planner_flags(m));
    }

    /// get FFTer for dimension m
    static ConvolvePlanR2R<T>& get_ffter(unsigned int m) {
        static map<unsigned int, ConvolvePlanR2R<T>*> ffters;
        std::lock_guard<std::mutex> lk(ConvolvePlan<T>::plannerLock());
        auto it = ffters.find(m);
        if(it != ffters.end()) return *(it->second);
        return *(ffters[m] = new Convolve_DCT_DST_I<T>(m));
    }

    /// normalization "logical size" for given input size
    unsigned int normSize() const override { return 2*(this->M-1); }
    /// multiply k-space kernel (with any appropriate shifts)
    void kmul(const vector<T>& k) override { for(size_t i=0; i<this->M-2; i++) this->kspace[i] = k[i]*this->kspace[i+1]; }
    /// assign k-space kernel vector
    void getkKern(vector<T>& k) const override { k.assign(this->kspace.data(), this->kspace.data() + this->M-2); }
    /// assign results to output vector
    void getResult(vector<T>& v) const override { v.assign(this->realspace.data(), this->realspace.data() + this->M-2); }
};

/// Convolution plan for DCT-II * DST-II -> DST-I
/// abcd, efgh ->  abcddcba * efghHGFE
template<typename T = double>
class Convolve_DCT_DST_II: public ConvolvePlanR2R<T> {
public:
    /// Constructor
    Convolve_DCT_DST_II(unsigned int m): ConvolvePlanR2R<T>(m) {
        this->d_fwd = this->plan_r2r_1d(m,   this->realspace.data(), this->kspace.data(), FFTW_REDFT10, planner_flags(m));
        this->k_fwd = this->plan_r2r_1d(m,   this->realspace.data(), this->kspace.data(), FFTW_RODFT10, planner_flags(m));
        this->p_rev = this->plan_r2r_1d(m-1, this->kspace.data(), this->realspace.data(), FFTW_RODFT00, planner_flags(m));
    }

    /// get FFTer for dimension m
    static ConvolvePlanR2R<T>& get_ffter(unsigned int m) {
        static map<unsigned int, ConvolvePlanR2R<T>*> ffters;
        std::lock_guard<std::mutex> lk(ConvolvePlan<T>::plannerLock());
        auto it = ffters.find(m);
        if(it != ffters.end()) return *(it->second);
        return *(ffters[m] = new Convolve_DCT_DST_II<T>(m));
    }

    /// multiply k-space kernel (with any appropriate shifts)
    void kmul(const vector<T>& k) override { for(size_t i=0; i<this->M-1; i++) this->kspace[i] = k[i]*this->kspace[i+1]; }
    /// assign results to output vector
    void getResult(vector<T>& v) const override { v.assign(this->realspace.data(), this->realspace.data() + this->M - 1); }
};


//-----------------------------------
//-----------------------------------
//-----------------------------------


/// Base class for convolver (Real-to-real symmetric data/kernel), cacheing intermediate results for re-use on same-sized vectors
template<typename T = double>
class ConvolverFactoryR2R: public fftwx<T> {
public:
    /// Polymorphic Destructor
    virtual ~ConvolverFactoryR2R() { }

    /// perform convolution (auto conversion of any data type)
    template<typename Vec_t>
    void convolve(Vec_t& v) {
        auto& kern = getKernel(v.size()); // get kernel first, since migh change FFTer
        auto& ffter = getPlan(v.size());
        std::copy(v.begin(), v.end(), ffter.realspace.begin());
        this->execute(ffter.d_fwd);
        ffter.kmul(kern);
        this->execute(ffter.p_rev);
        vector<T> vres;
        ffter.getResult(vres);
        v.assign(vres.begin(), vres.end());
    }

protected:
    /// get appropriate plan type
    virtual ConvolvePlanR2R<T>& getPlan(unsigned int i) const = 0;
    /// calculate real-space convolution kernel for given input size
    virtual vector<T> calcKernel(unsigned int i) const = 0;

    /// get (precalculated) k-space convolution kernel for input size
    const vector<T>& getKernel(unsigned int i) {
        auto it = kdata.find(i);
        if(it != kdata.end()) return it->second;

        auto& ffter = getPlan(i);
        auto kern = calcKernel(i);
        for(auto& k: kern) k /= ffter.normSize();

        for(auto& x: ffter.realspace) x = 0;
        std::copy(kern.begin(), kern.end(), ffter.realspace.begin());
        this->execute(ffter.k_fwd);

        auto& k = kdata[i];
        ffter.getkKern(k);
        return k;
    }

    map<unsigned int, vector<T>> kdata;    ///< convolutions for each array size
};

/// Gaussian convolutions generator
template<typename T = double>
class GaussConvolverFactory: public ConvolverFactoryR2R<T> {
public:
    /// Constructor
    GaussConvolverFactory(double rr): r(rr) { }
    const double r;     ///< convolution radius in samples
protected:
    /// get appropriate plan type
    ConvolvePlanR2R<T>& getPlan(unsigned int i) const override { return Convolve_DCT_I<T>::get_ffter(i); }
    /// calculate convolution kernel for given size
    vector<T> calcKernel(unsigned int i) const override {
        vector<T> v(i);
        double nrm = 0;
        for(int n=0; n<(int)i; n++) {
            v[n] = exp(-pow((n+0.5)/r,2)/2);
            nrm += (n? 2:1)*v[n];
        }
        for(auto& x: v) x /= nrm;
        return v;
    }
};

/// Base class for convolver, cacheing intermediate results for re-use on same-sized vectors
template<typename T = double>
class ConvolverFactoryR2C: public fftwx<T> {
public:
    /// Polymorphic Destructor
    virtual ~ConvolverFactoryR2C() { }

    /// perform convolution (auto conversion of any data type)
    template<typename Vec_t>
    void convolve(Vec_t& v) {
        auto& kern = getKernel(v.size()); // get kernel first, since migh change FFTer
        auto& ffter = ConvolvePlanR2C<T>::get_ffter(v.size());
        std::copy(v.begin(), v.end(), ffter.realspace);
        this->execute(ffter.d_fwd);
        for(size_t i=0; i<kern.size(); i++) ffter.kspace[i] *= kern[i];
        this->execute(ffter.p_rev);
        v.assign(ffter.realspace, ffter.realspace + v.size());
    }

protected:
    /// calculate real-space convolution kernel for given input size
    virtual vector<T> calcKernel(unsigned int i) const = 0;

    /// get (precalculated) k-space convolution kernel for input size
    const vector<std::complex<T>>& getKernel(unsigned int i) {
        auto it = kdata.find(i);
        if(it != kdata.end()) return it->second;

        auto& ffter = ConvolvePlanR2C<T>::get_ffter(i);
        auto kern = calcKernel(i);
        for(auto& k: kern) k /= ffter.normSize();

        std::copy(kern.begin(), kern.end(), ffter.realspace);
        this->execute(ffter.k_fwd);

        auto& v = kdata[i];
        v.assign(ffter.kspace, ffter.kspace+i/2+1);
        return v;
    }

    map<unsigned int, vector<std::complex<T>>> kdata;    ///< convolutions for each array size
};

#endif
