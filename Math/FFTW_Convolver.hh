/// \file FFTW_Convolver.hh Fast convolution utilities using FFTW3
// Michael P. Mendenhall, LLNL 2020

#ifndef FFTW_CONVOLVER_H
#define FFTW_CONVOLVER_H

#include "fftwx.hh"
#include <mutex>
#include <map>
using std::map;

//-----------------------------
//-----------------------------
//----- Convolution plans -----
//-----------------------------
//-----------------------------

/// Base class for precalculated convolution plan
template<typename T = double>
class ConvolvePlan: public fftwx<T> {
public:
    typedef typename fftwx<T>::plan_t plan_t;

    /// Constructor
    ConvolvePlan(size_t m): M(m), realspace(M) { }

    /// Polymorphic Destructor
    virtual ~ConvolvePlan() { }

    const size_t M; ///< number of elements (input)

    plan_t d_fwd;   ///< data real->kspace plan
    plan_t k_fwd;   ///< kernel real->kspace plan
    plan_t p_rev;   ///< convolved product kspace->real plan

    fftw_real_vec<T> realspace; ///< real-space side of transform data

    /// normalization "logical size" for given input size
    virtual size_t normSize() const { return 2*M; }
    /// output size after convolution
    virtual size_t outSize() const { return M; }
    /// k-space kernel size
    virtual size_t kkernSize() const { return M; }

    /// convenience function for planner flags
    virtual int planner_flags() const { return FFTW_PATIENT | FFTW_DESTROY_INPUT; }

    static std::mutex& plannerLock();   ///< lock on using FFTW to generate plans
};

template<>
std::mutex& ConvolvePlan<double>::plannerLock();
template<>
std::mutex& ConvolvePlan<float>::plannerLock();
template<>
std::mutex& ConvolvePlan<long double>::plannerLock();


/// Real-to-complex (periodic boundary conditions) convolution workspace
template<typename T = double>
class ConvolvePlanR2C: public ConvolvePlan<T> {
public:
    typedef typename fftwx<T>::fcplx_t fcplx_t;

    fftw_cplx_vec<T> kspace;    ///< kspace-side of transform data

    /// get precalculated convolution workspace for dimension m
    static ConvolvePlanR2C& get_ffter(size_t m) {
        static map<size_t, ConvolvePlanR2C<T>*> ffters;
        std::lock_guard<std::mutex> lk(ConvolvePlan<T>::plannerLock());
        auto it = ffters.find(m);
        if(it != ffters.end()) return *(it->second);
        return *(ffters[m] = new ConvolvePlanR2C<T>(m));
    }

    /// k-space kernel size
    size_t kkernSize() const override { return this->M/2 + 1; }
    /// normalization "logical size" for given input size
    size_t normSize() const override { return this->M; }

protected:
    /// Constructor
    ConvolvePlanR2C(size_t m): ConvolvePlan<T>(m), kspace(m/2+1) {
        this->d_fwd = this->plan_dft_r2c_1d(m, this->realspace.data(), (fcplx_t*)kspace.data(), this->planner_flags());
        this->k_fwd = this->plan_dft_r2c_1d(m, this->realspace.data(), (fcplx_t*)kspace.data(), this->planner_flags());
        this->p_rev = this->plan_dft_c2r_1d(m, (fcplx_t*)kspace.data(), this->realspace.data(), this->planner_flags());
    }
};


/// Real-to-real (symmetric boundary conditions) convolution workspace
template<typename T = double>
class ConvolvePlanR2R: public ConvolvePlan<T> {
public:

    /// multiply k-space kernel (with any appropriate shifts)
    virtual void kmul(const vector<T>& k) { for(size_t i=0; i<this->M; i++) kspace[i] *= k[i]; }

    fftw_real_vec<T> kspace;    ///< kspace-side of transform data

protected:
    /// Constructor
    ConvolvePlanR2R(size_t m): ConvolvePlan<T>(m), kspace(m) { }
};


/// Convolution plan for DCT-I * DCT-I -> DCT-I
/// abcd, efgh -> abcdcb * efghgf
template<typename T = double>
class Convolve_DCT_I: public ConvolvePlanR2R<T> {
public:
    /// get precalculated convolution workspace for dimension m
    static ConvolvePlanR2R<T>& get_ffter(size_t m) {
        static map<size_t,ConvolvePlanR2R<T>*> ffters;
        std::lock_guard<std::mutex> lk(ConvolvePlan<T>::plannerLock());
        auto it = ffters.find(m);
        if(it != ffters.end()) return *(it->second);
        return *(ffters[m] = new Convolve_DCT_I<T>(m));
    }

    /// normalization "logical size" for given input size
    size_t normSize() const override { return 2*(this->M-1); }

protected:
    /// Constructor
    Convolve_DCT_I(size_t m): ConvolvePlanR2R<T>(m) {
        this->d_fwd = this->plan_r2r_1d(m, this->realspace.data(), this->kspace.data(), FFTW_REDFT00, this->planner_flags());
        this->k_fwd = this->d_fwd;
        this->p_rev = this->plan_r2r_1d(m, this->kspace.data(), this->realspace.data(), FFTW_REDFT00, this->planner_flags());
    }
};


/// Convolution plan for DCT-I * DST-I -> DST-I
/// abcd, ef ->   abcdcb * ef0FE0
template<typename T = double>
class Convolve_DCT_DST_I: public ConvolvePlanR2R<T> {
public:
    /// get precalculated convolution workspace for dimension m
    static ConvolvePlanR2R<T>& get_ffter(size_t m) {
        static map<size_t, ConvolvePlanR2R<T>*> ffters;
        std::lock_guard<std::mutex> lk(ConvolvePlan<T>::plannerLock());
        auto it = ffters.find(m);
        if(it != ffters.end()) return *(it->second);
        return *(ffters[m] = new Convolve_DCT_DST_I<T>(m));
    }

    /// normalization "logical size" for given input size
    size_t normSize() const override { return 2*(this->M-1); }
    /// convolved output data size
    size_t outSize() const override { return this->M - 2; }
    /// k-space kernel size
    size_t kkernSize() const override { return this->M - 2; }

    /// multiply k-space kernel (with any appropriate shifts)
    void kmul(const vector<T>& k) override { for(size_t i=0; i<this->M-2; i++) this->kspace[i] = k[i]*this->kspace[i+1]; }

protected:
    /// Constructor
    Convolve_DCT_DST_I(size_t m): ConvolvePlanR2R<T>(m) {
        this->d_fwd = this->plan_r2r_1d(m,   this->realspace.data(), this->kspace.data(), FFTW_REDFT00, this->planner_flags());
        this->k_fwd = this->plan_r2r_1d(m-2, this->realspace.data(), this->kspace.data(), FFTW_RODFT00, this->planner_flags());
        this->p_rev = this->plan_r2r_1d(m-2, this->kspace.data(), this->realspace.data(), FFTW_RODFT00, this->planner_flags());
    }
};


/// Convolution plan for DCT-II * DST-II -> DST-I
/// abcd, efgh ->  abcddcba * efghHGFE
template<typename T = double>
class Convolve_DCT_DST_II: public ConvolvePlanR2R<T> {
public:
    /// get FFTer for dimension m
    static ConvolvePlanR2R<T>& get_ffter(size_t m) {
        static map<size_t, ConvolvePlanR2R<T>*> ffters;
        std::lock_guard<std::mutex> lk(ConvolvePlan<T>::plannerLock());
        auto it = ffters.find(m);
        if(it != ffters.end()) return *(it->second);
        return *(ffters[m] = new Convolve_DCT_DST_II<T>(m));
    }

    /// convolved output data size
    size_t outSize() const override { return this->M - 1; }

    /// multiply k-space kernel (with any appropriate shifts)
    void kmul(const vector<T>& k) override { for(size_t i=0; i<this->M-1; i++) this->kspace[i] = k[i]*this->kspace[i+1]; }

protected:
    /// Constructor
    Convolve_DCT_DST_II(size_t m): ConvolvePlanR2R<T>(m) {
        this->d_fwd = this->plan_r2r_1d(m,   this->realspace.data(), this->kspace.data(), FFTW_REDFT10, this->planner_flags());
        this->k_fwd = this->plan_r2r_1d(m,   this->realspace.data(), this->kspace.data(), FFTW_RODFT10, this->planner_flags());
        this->p_rev = this->plan_r2r_1d(m-1, this->kspace.data(), this->realspace.data(), FFTW_RODFT00, this->planner_flags());
    }
};


//-------------------------------
//-------------------------------
//----- Convolver factories -----
//-------------------------------
//-------------------------------


/// Base for convolver "factory," keeping pre-calculated kspace kernels
template<typename T>
class ConvolverFactory: public fftwx<T> {
public:
    /// Polymorphic Destructor
    virtual ~ConvolverFactory() { }

    /// Perform convolution
    template<typename Vec_t>
    void convolve(Vec_t& v) {
        prepareKernel(v.size());
        auto& P = getPlan(v.size());
        auto& vr = P.realspace;
        std::copy(v.begin(), v.end(), vr.begin());
        _convolve(v.size());
        v.assign(vr.begin(), vr.begin() + P.outSize());
    }

protected:
    /// calculate/cache kernel for specified size
    virtual void prepareKernel(size_t i) = 0;
    /// get planner for specified size
    virtual ConvolvePlan<T>& getPlan(size_t i) const = 0;
    /// calculate real-space convolution kernel for given input size
    virtual vector<T> calcKernel(size_t i) const = 0;
    /// perform convolution on pre-filled planned size
    virtual void _convolve(size_t i) = 0;
};

/// Real-to-real symmetric data/kernel convolver factory base
template<typename T = double>
class ConvolverFactoryR2R: public ConvolverFactory<T> {
protected:
    /// perform convolution on pre-filled planned size
    void _convolve(size_t i) override {
        auto& ffter = getR2RPlan(i);
        this->execute(ffter.d_fwd);
        ffter.kmul(kdata.at(i));
        this->execute(ffter.p_rev);
    }

    /// get generic planner
    ConvolvePlan<T>& getPlan(size_t i) const override { return getR2RPlan(i); }

    /// get appropriate plan type
    virtual ConvolvePlanR2R<T>& getR2RPlan(size_t i) const = 0;

    /// calculate/cache kernel for specified size
    void prepareKernel(size_t i) override {
        if(kdata.count(i)) return;

        auto& ffter = getR2RPlan(i);
        auto kern = this->calcKernel(i);
        for(auto& k: kern) k /= ffter.normSize();

        for(auto& x: ffter.realspace) x = 0;
        std::copy(kern.begin(), kern.end(), ffter.realspace.begin());
        this->execute(ffter.k_fwd);

        kdata[i].assign(ffter.kspace.begin(), ffter.kspace.begin() + ffter.kkernSize());
    }

    map<size_t, vector<T>> kdata; ///< cached kspace kernels for each input size
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
    ConvolvePlanR2R<T>& getR2RPlan(size_t i) const override { return Convolve_DCT_I<T>::get_ffter(i); }

    /// calculate convolution kernel for given size
    vector<T> calcKernel(size_t i) const override {
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

/// Real-to-complex (arbitrary periodic) convolver factory base
template<typename T = double>
class ConvolverFactoryR2C: public ConvolverFactory<T> {
protected:
    /// get appropriate-size convolver plan
    ConvolvePlan<T>& getPlan(size_t i) const override { return ConvolvePlanR2C<T>::get_ffter(i); }

    /// perform convolution on pre-filled planned size
    void _convolve(size_t i) override {
        auto& ffter = ConvolvePlanR2C<T>::get_ffter(i);
        auto& kern = kdata.at(i);
        this->execute(ffter.d_fwd);
        for(size_t n=0; n<kern.size(); ++n) ffter.kspace[n] *= kern[n];
        this->execute(ffter.p_rev);
    }

    /// prepare kernel for size
    void prepareKernel(size_t i) override {
        if(kdata.count(i)) return;
        auto& ffter = ConvolvePlanR2C<T>::get_ffter(i);
        auto kern = this->calcKernel(i);
        for(auto& k: kern) k /= ffter.normSize();

        std::copy(kern.begin(), kern.end(), ffter.realspace);
        this->execute(ffter.k_fwd);

        kdata[i].assign(ffter.kspace, ffter.kspace+i/2+1);
    }

    map<size_t, vector<std::complex<T>>> kdata; ///< cached kspace kernels for each input size
};

#endif
