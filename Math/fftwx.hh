/// @file fftwx.hh FFTW3 templated types helper
// -- Michael P. Mendenhall, LLNL 2020

#ifndef FFTWX_HH
#define FFTWX_HH

#include <complex>
#include <fftw3.h>
#include <limits>      // for std::numeric_limits
#include <type_traits> // for std::is_same
#include <mutex>
#include "fixed_vector.hh"

extern std::mutex fftw_planner_mutex;
#define FFTWLOCK std::lock_guard<std::mutex> _lk(fftw_planner_mutex)

/// placeholder for various FFTW data type operations
template<typename T>
struct fftwx { };

template<>
struct fftwx<double> {
    typedef double real_t;
    typedef fftw_plan plan_t;
    typedef fftw_complex fcplx_t;
    typedef std::complex<real_t> scplx_t;

    static real_t* alloc_real(size_t i) { FFTWLOCK; return static_cast<real_t*>(fftw_malloc(i*sizeof(real_t))); }
    static fcplx_t* alloc_complex(size_t i) { FFTWLOCK; return static_cast<fcplx_t*>(fftw_malloc(i*sizeof(fcplx_t))); }
    //static real_t* alloc_real(size_t i) { FFTWLOCK; return fftw_alloc_real(i); }
    //static fcplx_t* alloc_complex(size_t i) { FFTWLOCK; return fftw_alloc_complex(i); }
    static void free(void* p) { FFTWLOCK; fftw_free(p); }
    static void execute(plan_t& p) { fftw_execute(p); }

    template<typename... Args>
    static plan_t plan_dft_1d(Args&&... a) { FFTWLOCK; return fftw_plan_dft_1d(std::forward<Args>(a)...); }
    template<typename... Args>
    static plan_t plan_dft_r2c_1d(Args&&... a) { FFTWLOCK; return fftw_plan_dft_r2c_1d(std::forward<Args>(a)...); }
    template<typename... Args>
    static plan_t plan_dft_c2r_1d(Args&&... a) { FFTWLOCK; return fftw_plan_dft_c2r_1d(std::forward<Args>(a)...); }
    template<typename... Args>
    static plan_t plan_r2r_1d(Args&&... a) { FFTWLOCK; return fftw_plan_r2r_1d(std::forward<Args>(a)...); }
};

template<>
struct fftwx<float> {
    typedef float real_t;
    typedef fftwf_plan plan_t;
    typedef fftwf_complex fcplx_t;
    typedef std::complex<real_t> scplx_t;

    static real_t* alloc_real(size_t i) { FFTWLOCK; return static_cast<real_t*>(fftwf_malloc(i*sizeof(real_t))); }
    static fcplx_t* alloc_complex(size_t i) { FFTWLOCK; return static_cast<fcplx_t*>(fftwf_malloc(i*sizeof(fcplx_t))); }
    //static real_t* alloc_real(size_t i) { return fftwf_alloc_real(i); }
    //static fcplx_t* alloc_complex(size_t i) { return fftwf_alloc_complex(i); }
    static void free(void* p) { FFTWLOCK; fftwf_free(p); }
    static void execute(plan_t& p) { fftwf_execute(p); }

    template<typename... Args>
    static plan_t plan_dft_1d(Args&&... a) { FFTWLOCK; return fftwf_plan_dft_1d(std::forward<Args>(a)...); }
    template<typename... Args>
    static plan_t plan_dft_r2c_1d(Args&&... a) { FFTWLOCK; return fftwf_plan_dft_r2c_1d(std::forward<Args>(a)...); }
    template<typename... Args>
    static plan_t plan_dft_c2r_1d(Args&&... a) { FFTWLOCK; return fftwf_plan_dft_c2r_1d(std::forward<Args>(a)...); }
    template<typename... Args>
    static plan_t plan_r2r_1d(Args&&... a) { FFTWLOCK; return fftwf_plan_r2r_1d(std::forward<Args>(a)...); }
};

template<>
struct fftwx<long double> {
    typedef long double real_t;
    typedef fftwl_plan plan_t;
    typedef fftwl_complex fcplx_t;
    typedef std::complex<real_t> scplx_t;

    static real_t* alloc_real(size_t i) { FFTWLOCK; return static_cast<real_t*>(fftwl_malloc(i*sizeof(real_t))); }
    static fcplx_t* alloc_complex(size_t i) { FFTWLOCK; return static_cast<fcplx_t*>(fftwl_malloc(i*sizeof(fcplx_t))); }
    //static real_t* alloc_real(size_t i) { FFTWLOCK; return fftwl_alloc_real(i); }
    //static fcplx_t* alloc_complex(size_t i) { FFTWLOCK; return fftwl_alloc_complex(i); }
    static void free(void* p) { FFTWLOCK; fftwl_free(p); }
    static void execute(plan_t& p) { fftwl_execute(p); }

    template<typename... Args>
    static plan_t plan_dft_1d(Args&&... a) { FFTWLOCK; return fftwl_plan_dft_1d(std::forward<Args>(a)...); }
    template<typename... Args>
    static plan_t plan_dft_r2c_1d(Args&&... a) { FFTWLOCK; return fftwl_plan_dft_r2c_1d(std::forward<Args>(a)...); }
    template<typename... Args>
    static plan_t plan_dft_c2r_1d(Args&&... a) { FFTWLOCK; return fftwl_plan_dft_c2r_1d(std::forward<Args>(a)...); }
    template<typename... Args>
    static plan_t plan_r2r_1d(Args&&... a) { FFTWLOCK; return fftwl_plan_r2r_1d(std::forward<Args>(a)...); }

    static std::mutex planner_mutex;
};

#ifdef WITH_FFTW_FLOAT128

template<>
struct fftwx<__float128> {
    typedef __float128 real_t;
    typedef fftwq_plan plan_t;
    typedef fftwq_complex fcplx_t;
    typedef std::complex<real_t> scplx_t;

    static real_t* alloc_real(size_t i) { FFTWLOCK; return fftwq_alloc_real(i); }
    static fcplx_t* alloc_complex(size_t i) { FFTWLOCK; return fftwq_alloc_complex(i); }
    static void free(void* p) { FFTWLOCK; fftwq_free(p); }
    static void execute(plan_t& p) { fftwq_execute(p); }

    template<typename... Args>
    static plan_t plan_dft_1d(Args&&... a) { FFTWLOCK; return fftwq_plan_dft_1d(std::forward<Args>(a)...); }
    template<typename... Args>
    static plan_t plan_dft_r2c_1d(Args&&... a) { FFTWLOCK; return fftwq_plan_dft_r2c_1d(std::forward<Args>(a)...); }
    template<typename... Args>
    static plan_t plan_dft_c2r_1d(Args&&... a) { FFTWLOCK; return fftwq_plan_dft_c2r_1d(std::forward<Args>(a)...); }
    template<typename... Args>
    static plan_t plan_r2r_1d(Args&&... a) { FFTWLOCK; return fftwq_plan_r2r_1d(std::forward<Args>(a)...); }
};

#endif


#include <vector>
using std::vector;

/// real allocator wrapper for using FFTW allocate functions with std::vector
template <typename T>
struct fftwx_real_allocator {
    /// required for Allocator
    typedef T value_type;

    /// default constuctor
    fftwx_real_allocator() = default;

    /// interchangeable copy constructable
    constexpr fftwx_real_allocator(const fftwx_real_allocator&) noexcept {}
    /// interchangeable only with same type
    template<class U>
    constexpr bool operator==(const fftwx_real_allocator<U>&) const noexcept { return std::is_same<T,U>::value; }
    /// corresponding !=
    template<class U>
    constexpr bool operator!=(const fftwx_real_allocator<U>& o) const noexcept { return !(*this == o); }


    /// allocation, with bounds checks (required)
    value_type* allocate(std::size_t n) {
        if(n > std::numeric_limits<std::size_t>::max() / sizeof(value_type)) throw std::bad_alloc();
        if(auto p = fftwx<T>::alloc_real(n)) return p;
        throw std::bad_alloc();
    }

    /// deallocation (required)
    void deallocate(value_type* p, std::size_t) noexcept { fftwx<T>::free(p); }
};

/// complex allocator wrapper for using FFTW allocate functions with std::vector
template <typename T>
struct fftwx_complex_allocator {
    /// required for Allocator
    typedef typename fftwx<T>::scplx_t value_type;

    /// necessary since T != allocated type; else, compiler tries to define fftwx_complex_allocator<std::complex<T>>
    template <class U> struct rebind {
        typedef fftwx_complex_allocator<typename U::value_type> other;
    };

    /// default constuctor
    fftwx_complex_allocator() { }
    /// interchangeable copy constructable
    constexpr fftwx_complex_allocator(const fftwx_complex_allocator&) noexcept { }

    /// interchangeable only with same type
    template<class U>
    constexpr bool operator==(const fftwx_complex_allocator<U>&) const noexcept { return std::is_same<T,U>::value; }
    /// corresponding !=
    template<class U>
    constexpr bool operator!=(const fftwx_complex_allocator<U>& o) const noexcept { return !(*this == o); }

    /// allocation, with bounds checks (required)
    value_type* allocate(std::size_t n) {
        if(n > std::numeric_limits<std::size_t>::max() / sizeof(value_type)) throw std::bad_alloc();
        if(auto p = fftwx<T>::alloc_complex(n)) return (value_type*)p;
        throw std::bad_alloc();
    }

    /// deallocation (required)
    void deallocate(value_type* p, std::size_t) noexcept { fftwx<T>::free(p); }
};

/// FFTW-allocated real data
template <typename T>
using fftw_real_vec = fixed_vector<T, fftwx_real_allocator<T>>;

/// FFTW-allocated complex data
template <typename T>
using fftw_cplx_vec = fixed_vector<typename fftwx<T>::scplx_t, fftwx_complex_allocator<T>>;

#endif
