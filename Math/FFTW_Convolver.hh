/// \file FFTW_Convolver.hh Fast convolution utilities using FFTW3
// -- Michael P. Mendenhall, LLNL 2020

/* ***********************************************************************

-------------------------
-- FFTW Real DFT notes --
-------------------------

General case: N real entries -> N/2 + 1 uniquely-determined
complex entries, symmetric by complex conjugate around center.


Example: [N = n = 4, even]
a b c d  < x-space input
M u V u* < k-space: N//2 + 1 = 3 unique entries (M real, u complex, V == V* real)
0 1 2    < k-space outputs

Example: [N = n = 5, odd]
a b c d e   < x-space input
M u v v*u*  < k-space: N//2 + 1 = 3 unique entries (M real, u,v complex)
0 1 2       < k-space outpus

-------------------
- Additional symmetries reflected in k-space results
-------------------

n = number of elements to uniquely determine pattern, given symmetries ("physical dimension")
N = periodicity size ("logical dimension")

-------------------------------------
-------------------------------------
For even-symmetry inputs, k-space is real and even.

DCT-I [REDFT00] N = 2*(n-1), even around j=0 and even around j=n-1.
0 1 2 3 4        < inputs
a b c d e d c b  < implied logical
u v w x y x w v  < k-space
0 1 2 3 4        < outputs

DCT-II [REDFT10] N = 2*n, ("the" DCT) even around j=-0.5 and even around j=n-0.5.
  0   1   2               < inputs
0 a 0 b 0 c 0 c 0 b 0 a   < implied logical
u v w 0 W V U V W 0 w v   < k-space
0 1 2                     < outputs

DCT-III [REDFT01] N = 4*n, ("the" IDCT) even around j=0 and odd around j=n.
0 1 2                     < inputs
a b c 0 C B A B C 0 c b   < implied
0 u 0 v 0 w w 0 v 0 u 0   < k-space
  0   1   2               < outputs
n = number of elements to uniquely determine pattern, given symmetries ("physical dimension")

DCT-IV [REDFT11] N = 4*n, even around j=-0.5 and odd around j=n-0.5.
  0   1   2                                     < inputs
0 a 0 b 0 c 0 C 0 B 0 A 0 A 0 B 0 C 0 c 0 b 0 a < implied
0 u 0 v 0 w 0 W 0 V 0 U 0|U 0 V 0 W 0 w 0 v 0 u < k-space
  0   1   2                                     < outputs

-------------------------------------
-------------------------------------
For odd-symmetry inputs, k-space is imaginary and odd.

DST-I [RODFT00] N = 2*(n+1), odd around j=-1 and odd around j=n.
  0 1 2             < inputs
0 a b c 0 C B A     < implied
0 u v w 0 W V U     < k-space
  0 1 2             < outputs

DST-II [RODFT10] N = 2*n, odd around j=-0.5 and odd around j=n-0.5.
  0   1   2                 < inputs
0 a 0 b 0 c 0 C 0 B 0 A     < implied
0 u v w v u 0 U V W V U     < k-space
  0 1 2                     < outputs

DST-III [RODFT01] N = 4*n, odd around j=-1 and even around j=n-1.
  0 1 2                     < inputs
0 a b c b a 0 A B C B A     < implied
0 u 0 v 0 w 0 W 0 V 0 U     < k-space
  0   1   2                 < outputs

DST-IV [RODFT11] N = 4*n, odd around j=-0.5 and even around j=n-0.5
  0   1   2                                         < inputs
0 a 0 b 0 c 0 c 0 b 0 a 0 A 0 B 0 C 0 C 0 B 0 A     < implied
0 a 0 b 0 c 0 c 0 b 0 a 0 A 0 B 0 C 0 C 0 B 0 A     < k-space
  0   1   2                                         < outputs


**************************************************************** */


#ifndef FFTW_CONVOLVER_H
#define FFTW_CONVOLVER_H

#include "fftwx.hh"

#include <cmath>
#include <map>
using std::map;


//-----------------------------------------
//----- Symmetrizing helper utilities -----
//-----------------------------------------

/// symmetrize around center element abcd -> abc d cb
template<class V>
V symmetrize_o(const V& v) {
    V v2(v);
    for(int i = int(v.size()-2); i > 0; --i) v2.push_back(v[i]);
    return v2;
}

/// mirror-symmetrize abc -> abc cba
template<class V>
V symmetrize_e(const V& v) {
    V v2(v);
    for(int i = int(v.size()-1); i >= 0; --i) v2.push_back(v[i]);
    return v2;
}

/// zero-interleave (half-samples)
template<class V>
V interzero(const V& v) {
    V v2;
    for(auto x: v) { v2.push_back(0); v2.push_back(x); }
    return v2;
}

/// antisymmetrize abc -> abc BCA
template<class V>
V asymmetrize_e(const V& v) {
    V v2(v);
    for(int i = int(v.size()-1); i >= 0; --i) v2.push_back(-v[i]);
    return v2;
}

/// antisymmetrize abc -> abc 0 CBA [0]
template<class V>
V asymmetrize_o(const V& v, bool fzero = false) {
    V v2;
    if(fzero) v2.push_back(0);
    for(auto x: v) v2.push_back(x);
    v2.push_back(0);
    for(int i = int(v.size()-1); i >= 0; --i) v2.push_back(-v[i]);
    return v2;
}

/// duplicate negated abc -> abc ABC
template<class V>
V dupneg(const V& v) {
    V v2(v);
    for(auto x: v) v2.push_back(-x);
    return v2;
}

/// DST-III symmetry abc -> 0 abcba 0 ABCBA
template<class V>
V dstIIIsymm(const V& v) {
    V v2;
    v2.push_back(0);
    for(auto x: v) v2.push_back(x);
    for(int i = int(v.size()-2); i >= 0; --i) v2.push_back(v[i]);
    return dupneg(v2);
}

/// DST-IV symmetry
template<class V>
V dstIVsymm(const V& v) { return dupneg(interzero(symmetrize_e(v))); }


//-----------------
//-----------------
//----- Plans -----
//-----------------
//-----------------

/// Plan with workspace size info
template<typename T>
class TransformPlan: public fftwx<T> {
public:
    typedef typename fftwx<T>::plan_t plan_t;

    /// Constructor
    TransformPlan(size_t m, size_t nl, size_t k):
    M(m), Nlog(nl), K(k) { }

    /// Polymorphic Destructor
    virtual ~TransformPlan() { }

    /// execute plan
    void execute() { fftwx<T>::execute(p); }

    plan_t p;           ///< plan

    const size_t M;     ///< input array size
    const size_t Nlog;  ///< logical (normalization) size
    const size_t K;     ///< output array size

protected:
    /// convenience function for planner flags
    virtual int planner_flags() const { return FFTW_PATIENT | FFTW_DESTROY_INPUT; }
};

/// 1D (complex-to-complex) Discrete Fourier Transform plan
template<typename T = double>
class DFTPlan: public TransformPlan<T> {
protected:
    typedef typename fftwx<T>::fcplx_t xspace_t;
    typedef typename fftwx<T>::fcplx_t kspace_t;
    typedef fftw_cplx_vec<T> xvec_t;
    typedef fftw_cplx_vec<T> kvec_t;

    /// Constructor
    explicit DFTPlan(size_t m): TransformPlan<T>(m, m, m) { }

    /// make plan
    void makePlan(bool fwd, xspace_t* v_x, kspace_t* v_k) {
        if(fwd) this->p = this->plan_dft_1d(this->M, v_x, v_k, FFTW_FORWARD, this->planner_flags());
        else    this->p = this->plan_dft_1d(this->M, v_k, v_x, FFTW_BACKWARD, this->planner_flags());
    }
};

/// 1D real-to-complex plan
template<typename T = double>
class R2CPlan: public TransformPlan<T> {
public:
    typedef typename fftwx<T>::real_t xspace_t;
    typedef typename fftwx<T>::fcplx_t kspace_t;
    typedef fftw_real_vec<T> xvec_t;
    typedef fftw_cplx_vec<T> kvec_t;

    /// constructor
    explicit R2CPlan(size_t m): TransformPlan<T>(m, m, m/2+1) { }

    /// make plan
    void makePlan(bool fwd, xspace_t* v_x, kspace_t* v_k) {
        if(fwd) this->p = this->plan_dft_r2c_1d(this->M, v_x, v_k, this->planner_flags());
        else    this->p = this->plan_dft_c2r_1d(this->M, v_k, v_x, this->planner_flags());
    }
};

/// 1D real-to-real plan base
template<typename T = double>
class R2RPlan: public TransformPlan<T> {
public:
    typedef typename fftwx<T>::real_t xspace_t;
    typedef typename fftwx<T>::real_t kspace_t;
    typedef fftw_real_vec<T> xvec_t;
    typedef fftw_real_vec<T> kvec_t;

    /// Constructor
    R2RPlan(size_t m, size_t nl): TransformPlan<T>(m, nl, m) { }
};

//-----------------
//-- R2R variations
//-----------------

/// DCT-I real-to-real transform
template<typename T>
class DCT_I_Plan: public R2RPlan<T> {
public:
    /// Constructor
    explicit DCT_I_Plan(size_t m): R2RPlan<T>(m, 2*(m-1)) { }

    /// make plan
    void makePlan(bool fwd, T* v_x, T* v_k) {
        if(fwd) this->p = this->plan_r2r_1d(this->M, v_x, v_k, FFTW_REDFT00, this->planner_flags());
        else    this->p = this->plan_r2r_1d(this->K, v_k, v_x, FFTW_REDFT00, this->planner_flags());
    }
};

/// DCT-II real-to-real transform
template<typename T>
class DCT_II_Plan: public R2RPlan<T> {
public:
    /// Constructor
    explicit DCT_II_Plan(size_t m): R2RPlan<T>(m, 2*m) { }

    /// make plan
    void makePlan(bool fwd, T* v_x, T* v_k) {
        if(fwd) this->p = this->plan_r2r_1d(this->M, v_x, v_k, FFTW_REDFT10, this->planner_flags());
        else    this->p = this->plan_r2r_1d(this->K, v_k, v_x, FFTW_REDFT01, this->planner_flags());
    }
};

/// DCT-III real-to-real transform
template<typename T>
class DCT_III_Plan: public R2RPlan<T> {
public:
    /// Constructor
    explicit DCT_III_Plan(size_t m): R2RPlan<T>(m, 2*m) { }

    /// make plan
    void makePlan(bool fwd, T* v_x, T* v_k) {
        if(fwd) this->p = this->plan_r2r_1d(this->M, v_x, v_k, FFTW_REDFT01, this->planner_flags());
        else    this->p = this->plan_r2r_1d(this->K, v_k, v_x, FFTW_REDFT10, this->planner_flags());
    }
};


/// DCT-IV real-to-real transform
template<typename T>
class DCT_IV_Plan: public R2RPlan<T> {
public:
    /// Constructor
    explicit DCT_IV_Plan(size_t m): R2RPlan<T>(m, 2*m) { }

    /// make plan
    void makePlan(bool fwd, T* v_x, T* v_k) {
        if(fwd) this->p = this->plan_r2r_1d(this->M, v_x, v_k, FFTW_REDFT11, this->planner_flags());
        else    this->p = this->plan_r2r_1d(this->K, v_k, v_x, FFTW_REDFT11, this->planner_flags());
    }
};

/// DST-I real-to-real transform
template<typename T>
class DST_I_Plan: public R2RPlan<T> {
public:
    /// Constructor
    explicit DST_I_Plan(size_t m): R2RPlan<T>(m, 2*(m+1)) { }

    /// make plan
    void makePlan(bool fwd, T* v_x, T* v_k) {
        if(fwd) this->p = this->plan_r2r_1d(this->M, v_x, v_k, FFTW_RODFT00, this->planner_flags());
        else    this->p = this->plan_r2r_1d(this->K, v_k, v_x, FFTW_RODFT00, this->planner_flags());
    }
};

/// DST-II real-to-real transform
template<typename T>
class DST_II_Plan: public R2RPlan<T> {
public:
    /// Constructor
    explicit DST_II_Plan(size_t m): R2RPlan<T>(m, 2*m) { }

    /// make plan
    void makePlan(bool fwd, T* v_x, T* v_k) {
        if(fwd) this->p = this->plan_r2r_1d(this->M, v_x, v_k, FFTW_RODFT10, this->planner_flags());
        else    this->p = this->plan_r2r_1d(this->K, v_k, v_x, FFTW_RODFT01, this->planner_flags());
    }
};

/// DST-III real-to-real transform
template<typename T>
class DST_III_Plan: public R2RPlan<T> {
public:
    /// Constructor
    explicit DST_III_Plan(size_t m): R2RPlan<T>(m, 2*m) { }

    /// make plan
    void makePlan(bool fwd, T* v_x, T* v_k) {
        if(fwd) this->p = this->plan_r2r_1d(this->M, v_x, v_k, FFTW_RODFT01, this->planner_flags());
        else    this->p = this->plan_r2r_1d(this->K, v_k, v_x, FFTW_RODFT10, this->planner_flags());
    }
};

/// DST-IV real-to-real transform
template<typename T>
class DST_IV_Plan: public R2RPlan<T> {
public:
    /// Constructor
    explicit DST_IV_Plan(size_t m): R2RPlan<T>(m, 2*m) { }

    /// make plan
    void makePlan(bool fwd, T* v_x, T* v_k) {
        if(fwd) this->p = this->plan_r2r_1d(this->M, v_x, v_k, FFTW_RODFT11, this->planner_flags());
        else    this->p = this->plan_r2r_1d(this->K, v_k, v_x, FFTW_RODFT11, this->planner_flags());
    }
};



//----------------------
//----------------------
//----- Workspaces -----
//----------------------
//----------------------

/// Workspace wrapper for plan, plus cache of pre-calculated workspaces
template<class Plan>
class FFTWorkspace: public Plan {
public:
    typename Plan::xvec_t v_x;  ///< x-space
    typename Plan::kvec_t v_k;  ///< k-space

    /// Constructor
    FFTWorkspace(size_t m, bool fwd): Plan(m), v_x(Plan::M), v_k(Plan::K) {
        this->makePlan(fwd,
                       (typename Plan::xspace_t*)v_x.data(),
                       (typename Plan::kspace_t*)v_k.data());
    }

    /// get precalculated FFT workspace for dimension m
    static FFTWorkspace& get_ffter(size_t m, bool fwd) {
        static thread_local map<size_t, FFTWorkspace*> ffters[2];
        auto it = ffters[fwd].find(m);
        if(it != ffters[fwd].end()) return *(it->second);
        return *(ffters[fwd][m] = new FFTWorkspace(m,fwd));
    }
};

/// Conjugate forward/reverse pair
template<class Plan>
class IFFTWorkspace: public FFTWorkspace<Plan> {
public:
    /// Constructor
    explicit IFFTWorkspace(size_t m): FFTWorkspace<Plan>(m, true), p_rev(m) {
        p_rev.makePlan(false,
                       (typename Plan::xspace_t*)this->v_x.data(),
                       (typename Plan::kspace_t*)this->v_k.data());
    }

    /// execute reverse, with normalization
    void etucexe() {
        p_rev.execute();
        for(auto& x: this->v_x) x /= this->Nlog;
    }

    /// get precalculated FFT workspace pair
    static IFFTWorkspace& get_iffter(size_t m) {
        static thread_local map<size_t, IFFTWorkspace*> iffters;
        auto it = iffters.find(m);
        if(it != iffters.end()) return *(it->second);
        return *(iffters[m] = new IFFTWorkspace(m));
    }

    Plan p_rev; ///< reverse plan
};

//-----------------------------
//-----------------------------
//----- Convolution plans -----
//-----------------------------
//-----------------------------

/// Base class for precalculated convolution scheme, combining data forward-transform DP, forward kernel plan KP, and reverse output reverse RP
template<class DP, class KP, class RP>
class ConvolvePlan: public FFTWorkspace<DP> {
public:
    typedef DP FwdPlan_t;
    typedef KP KernPlan_t;
    typedef RP RevPlan_t;
    typedef FFTWorkspace<FwdPlan_t> WS;
    typedef typename WS::xspace_t xspace_t;
    typedef typename WS::kspace_t kspace_t;
    typedef typename WS::kvec_t kvec_t;


    /// Constructor
    explicit ConvolvePlan(size_t m, size_t _km = 0, size_t _rm = 0):
    WS(m, true), kernPlan(_km? _km : m), revPlan(_rm? _rm : m) {
        kernPlan.makePlan(true, (xspace_t*)this->v_x.data(), (kspace_t*)this->v_k.data());
        revPlan.makePlan(false, (xspace_t*)this->v_x.data(), (kspace_t*)this->v_k.data());
    }

    KernPlan_t kernPlan;    ///< kernel transform v_x -> v_k
    RevPlan_t revPlan;      ///< reverse transform v_k * kernel -> v_x

    /// multiply k-space kernel (with any appropriate shifts) --- specialize as needed
    virtual void kmul(const kvec_t& k) {
        if(k.size() != this->v_k.size() || k.size() != this->K) throw std::logic_error("Mismatched k-space kernel size");
        auto it = this->v_k.begin();
        for(auto x: k) *(it++) *= x;
    }

    /// perform convolution using pre-calculated k-space kernel
    void kconvolve(const kvec_t& kkern) {
        this->execute();
        kmul(kkern);
        revPlan.execute();
    }

    /// load input data
    template<typename V>
    void load(const V& v) {
        if(v.size() > this->M) throw std::logic_error("Mismatched convolution input");
        std::copy(v.begin(), v.end(), this->v_x.begin());
    }

    /// calculate (pre-normalized) k-space kernel
    template<typename V>
    void calcKkern(const V& k) {
        if(k.size() != kernPlan.M) throw std::logic_error("Mismatched convolution kernel size");
        load(k);
        kernPlan.execute();
        for(auto& x: this->v_k) x /= kernPlan.Nlog;
    }

    /// fetch real-space result
    template<typename V>
    void fetch(V& v, size_t kshift = 0) const {
        if(kshift) kshift = kshift % this->M;
        auto p0 = this->v_x.begin() + kshift;
        v.assign(p0, this->v_x.begin() + revPlan.M);
        if(kshift) v.insert(v.end(), this->v_x.begin(), p0);
    }

    /// full convolution sequence
    template<typename V, typename U>
    void convolve(V& v, const U& k) {
        calcKkern(k);
        auto kk = this->v_k;
        load(v);
        kconvolve(kk);
        fetch(v);
    }
};

template<typename T = double>
using ConvolvePlanR2C = ConvolvePlan<R2CPlan<T>, R2CPlan<T>, R2CPlan<T>>;

template<typename T>
using Convolve_DCT_I = ConvolvePlan<DCT_I_Plan<T>, DCT_I_Plan<T>, DCT_I_Plan<T>>;

template<typename T>
using Convolve_DCT_II = ConvolvePlan<DCT_II_Plan<T>, DCT_II_Plan<T>, DCT_II_Plan<T>>;

template<typename T>
using _Convolve_DCT_DST_I = ConvolvePlan<DCT_I_Plan<T>, DST_I_Plan<T>, DST_I_Plan<T>>;

template<typename T>
class Convolve_DCT_DST_I: public _Convolve_DCT_DST_I<T> {
public:
    /// Constructor
    explicit Convolve_DCT_DST_I(size_t m): _Convolve_DCT_DST_I<T>(m, m-2, m-2) { }
    /// multiply k-space kernel (with any appropriate shifts)
    void kmul(const typename _Convolve_DCT_DST_I<T>::kvec_t& k) override { for(size_t i=0; i<this->M-2; i++) this->v_k[i] = k[i] * this->v_k[i+1]; }
};

template<typename T>
using _Convolve_DCT_DST_II = ConvolvePlan<DCT_II_Plan<T>, DST_II_Plan<T>, DST_I_Plan<T>>;

/// Convolution plan for DCT-II * DST-II -> DST-I
/// abcd, efgh ->  abcddcba * efghHGFE = uvw0WVU0
template<typename T>
class Convolve_DCT_DST_II: public _Convolve_DCT_DST_II<T> {
public:
    /// Constructor
    explicit Convolve_DCT_DST_II(size_t m): _Convolve_DCT_DST_II<T>(m, m, m-1) { }
    /// multiply k-space kernel (with any appropriate shifts)
    void kmul(const typename _Convolve_DCT_DST_II<T>::kvec_t& k) override { for(size_t i=0; i<this->M-1; i++) this->v_k[i] = k[i]*this->v_k[i+1]; }
};


//-------------------------------
//-------------------------------
//----- Convolver factories -----
//-------------------------------
//-------------------------------

/// Base for convolver "factory," storing pre-calculated kspace kernels
template<class _C>
class ConvolverFactory {
public:
    typedef _C Convolver_t;

    /// Polymorphic Destructor
    virtual ~ConvolverFactory() { }

    /// Perform convolution
    template<typename Vec_t>
    void convolve(Vec_t& v) {
        prepareKernel(v.size());
        auto& C = getConvolver(v.size());
        C.load(v);
        C.kconvolve(kdata.at(v.size()));
        C.fetch(v, kshift(v.size()));
    }

    /// Generate appropriately-sized convolver
    static Convolver_t& getConvolver(size_t m) {
        static thread_local map<size_t, Convolver_t*> cs;
        auto it = cs.find(m);
        if(it != cs.end()) return *it->second;
        return *(cs[m] = new Convolver_t(m));
    }

protected:
    /// calculate real-space convolution kernel for given input size
    virtual vector<typename Convolver_t::xspace_t> calcKernel(size_t i) = 0;
    /// kshift to assign for kernel size
    virtual size_t kshift(size_t) const { return 0; }

    /// calculate/cache kernel for specified size
    void prepareKernel(size_t i) {
        if(kdata.count(i)) return;
        auto& C = getConvolver(i);
        C.calcKkern(this->calcKernel(i));
        kdata.emplace(i, C.v_k);
    }

    map<size_t, typename Convolver_t::kvec_t> kdata;  ///< cached kspace kernels for each input size
};

/// Gaussian convolutions generator, symmetrizing boundary conditions
template<typename T = double>
class GaussConvolverFactory: public ConvolverFactory<Convolve_DCT_I<T>> {
public:
    /// Constructor
    explicit GaussConvolverFactory(double _r): r(_r) { }
    const double r;     ///< convolution radius in samples

protected:
    /// calculate convolution kernel for given size
    vector<T> calcKernel(size_t i) override {
        vector<T> v(i);
        double nrm = 0;
        for(int n=0; n<(int)i; ++n) {
            v[n] = exp(-n*n/(2*r*r));
            nrm += (n? 2 : 1)*v[n];
        }
        for(auto& x: v) x /= nrm;
        return v;
    }
};

/// Gaussian-smoothed derivative filter; symmetrizing boundary conditions
template<typename T = double>
class GaussDerivFactory: public ConvolverFactory<Convolve_DCT_DST_II<T>> {
public:
    /// Constructor
    explicit GaussDerivFactory(T _r): r(_r) { }
    const T r; ///< convolution radius in samples

protected:
    /// calculate convolution kernel for given size
    vector<T> calcKernel(size_t i) override {
        while(verf.size() < i + 2) {
            int j = verf.size();
            verf.push_back(std::erf((long double)((j-0.5)/(sqrt(2.)*r))));
        }

        while(vkern.size() < i) {
            auto j = vkern.size();
            vkern.push_back(-0.5*(-verf[j]+2*verf[j+1]-verf[j+2]));
        }

        return vector<T>(vkern.begin(), vkern.begin() + i);
    }

    vector<T> verf;     ///< erfs at bin edges for integrals of Gaussian in bins
    vector<T> vkern;    ///< smoothing kernel * {-1,1} derivative
};

#endif
