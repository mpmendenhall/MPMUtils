/// \file FFTW_Convolver.cxx
// Michael P. Mendenhall, LLNL 2019

#ifndef WITHOUT_FFTW
#include "FFTW_Convolver.hh"

std::mutex ConvolvePlan::fftLock;

//////////////////
//////////////////

ConvolvePlanR2C::ConvolvePlanR2C(size_t m): ConvolvePlan(m),
realspace(M), kspace(M/2+1) {
    d_fwd = fftw_plan_dft_r2c_1d(M, realspace.data(), (fftw_complex*)kspace.data(), FFTW_PATIENT);
    k_fwd = fftw_plan_dft_r2c_1d(M, realspace.data(), (fftw_complex*)kspace.data(), FFTW_PATIENT);
    p_rev = fftw_plan_dft_c2r_1d(M, (fftw_complex*)kspace.data(), realspace.data(), FFTW_PATIENT);
}

ConvolvePlanR2C& ConvolvePlanR2C::get_ffter(size_t m) {
    std::lock_guard<std::mutex> lk(fftLock);
    static map<size_t,ConvolvePlanR2C*> ffters;
    auto it = ffters.find(m);
    if(it != ffters.end()) return *(it->second);
    return *(ffters[m] = new ConvolvePlanR2C(m));
}

//////////////////
//////////////////

Convolve_DCT_I::Convolve_DCT_I(size_t m): ConvolvePlanR2R(m) {
    d_fwd = fftw_plan_r2r_1d(M, realspace.data(), kspace.data(),
                             FFTW_REDFT00, FFTW_PATIENT);
    k_fwd = d_fwd;
    p_rev = fftw_plan_r2r_1d(M, kspace.data(), realspace.data(),
                             FFTW_REDFT00, FFTW_PATIENT);
}

ConvolvePlanR2R& Convolve_DCT_I::get_ffter(size_t m) {
    std::lock_guard<std::mutex> lk(fftLock);
    static map<size_t, ConvolvePlanR2R*> ffters;
    auto it = ffters.find(m);
    if(it != ffters.end()) return *(it->second);
    return *(ffters[m] = new Convolve_DCT_I(m));
}

//////////////////
//////////////////

Convolve_DCT_DST_I::Convolve_DCT_DST_I(size_t m): ConvolvePlanR2R(m) {
    d_fwd = fftw_plan_r2r_1d(M, realspace.data(), kspace.data(),
                             FFTW_REDFT00, FFTW_PATIENT);
    k_fwd = fftw_plan_r2r_1d(M-2, realspace.data(), kspace.data(),
                             FFTW_RODFT00, FFTW_PATIENT);
    p_rev = fftw_plan_r2r_1d(M-2, kspace.data(), realspace.data(),
                             FFTW_RODFT00, FFTW_PATIENT);
}

ConvolvePlanR2R& Convolve_DCT_DST_I::get_ffter(size_t m) {
    std::lock_guard<std::mutex> lk(fftLock);
    static map<size_t, ConvolvePlanR2R*> ffters;
    auto it = ffters.find(m);
    if(it != ffters.end()) return *(it->second);
    return *(ffters[m] = new Convolve_DCT_DST_I(m));
}

///////////////////
///////////////////

Convolve_DCT_DST_II::Convolve_DCT_DST_II(size_t m): ConvolvePlanR2R(m) {
    d_fwd = fftw_plan_r2r_1d(M, realspace.data(), kspace.data(),
                             FFTW_REDFT10, FFTW_PATIENT);
    k_fwd = fftw_plan_r2r_1d(M, realspace.data(), kspace.data(),
                             FFTW_RODFT10, FFTW_PATIENT);
    p_rev = fftw_plan_r2r_1d(M-1, kspace.data(), realspace.data(),
                             FFTW_RODFT00, FFTW_PATIENT);
}

ConvolvePlanR2R& Convolve_DCT_DST_II::get_ffter(size_t m) {
    std::lock_guard<std::mutex> lk(fftLock);
    static map<size_t, ConvolvePlanR2R*> ffters;
    auto it = ffters.find(m);
    if(it != ffters.end()) return *(it->second);
    return *(ffters[m] = new Convolve_DCT_DST_II(m));
}

///////////////////
///////////////////

void ConvolverFactoryR2R::convolve(vector<double>& v) {
    auto& kern = getKernel(v.size()); // get kernel first, since might cause construction of fft'er

    auto& ffter = getPlan(v.size());
    vector<double> rs = v;
    vector<double> ks(v.size());

    fftw_execute_r2r(ffter.d_fwd, rs.data(), ks.data());
    ffter.kmul(kern.data(), ks.data());
    fftw_execute_r2r(ffter.p_rev, ks.data(), rs.data());
    ffter.getResult(v, rs.data());
}

const vector<double>& ConvolverFactoryR2R::getKernel(size_t i) {
    std::lock_guard<std::mutex> lk(kernLock);

    auto it = kdata.find(i);
    if(it != kdata.end()) return it->second;

    auto& ffter = getPlan(i);
    std::fill(ffter.realspace.data(), ffter.realspace.data()+ffter.M, 0.);
    calcKernel(i, ffter.realspace.data());
    for(size_t j=0; j<ffter.M; j++) ffter.realspace[j] /= ffter.normSize();

    auto& k = kdata[i] = vector<double>(i);
    fftw_execute_r2r(ffter.k_fwd, ffter.realspace.data(), k.data());
    return k;
}

/////////////////////
/////////////////////

void GaussConvolverFactory::calcKernel(size_t i, double* v) const {
    double nrm = 0;
    for(size_t n=0; n<i; n++) {
        v[n] = exp(-pow((n+0.5)/r,2)/2);
        nrm += (n? 2:1)*v[n];
    }
    for(size_t n = 0; n < i; n++) v[n] /= nrm;
}

///////////////////
///////////////////

void ConvolverFactoryR2C::convolve(vector<double>& v) {
    auto N = v.size();
    auto kern = getKernel(N); // get kernel first, since might change FFTer
    fftwvec_r rs(N);
    fftwvec_c ks(N/2+1);

    auto& ffter = ConvolvePlanR2C::get_ffter(v.size());
    std::copy(v.begin(), v.end(), rs.data());
    fftw_execute_dft_r2c(ffter.d_fwd, rs.data(), (fftw_complex*)ks.data());
    for(size_t i=0; i<N/2+1; i++) ks[i] *= kern[i];
    fftw_execute_dft_c2r(ffter.p_rev, (fftw_complex*)ks.data(), rs.data());
    v.assign(rs.begin(), rs.end());
}

const vector<cplx_t>& ConvolverFactoryR2C::getKernel(size_t i) {
    std::lock_guard<std::mutex> lk(kernLock);

    auto it = kdata.find(i);
    if(it != kdata.end()) return it->second;

    auto& ffter = ConvolvePlanR2C::get_ffter(i);
    std::fill(ffter.realspace.data(), ffter.realspace.data()+i, 0.);
    calcKernel(i, ffter.realspace.data());
    for(size_t j=0; j<i; j++) ffter.realspace[j] /= ffter.normSize();

    auto& k = kdata[i] = vector<cplx_t>(i/2+1);
    fftw_execute_dft_r2c(ffter.k_fwd, ffter.realspace.data(), (fftw_complex*)k.data());
    return k;
}

#endif
