/// \file ConvolverFactory.cc
// Michael P. Mendenhall

#include "ConvolverFactory.hh"

ConvolvePlanR2C::ConvolvePlanR2C(unsigned int m): ConvolvePlan(m),
realspace(fftw_alloc_real(M)), kspace((std::complex<double>*)fftw_alloc_complex(M/2+1)) {
    assert(realspace && kspace);
    d_fwd = fftw_plan_dft_r2c_1d(M, realspace, (fftw_complex*)kspace, FFTW_PATIENT);
    k_fwd = fftw_plan_dft_r2c_1d(M, realspace, (fftw_complex*)kspace, FFTW_PATIENT);
    p_rev = fftw_plan_dft_c2r_1d(M, (fftw_complex*)kspace, realspace, FFTW_PATIENT);
}

map<unsigned int,ConvolvePlanR2C*> ConvolvePlanR2C::ffters;

ConvolvePlanR2C& ConvolvePlanR2C::get_ffter(unsigned int m) {
    auto it = ffters.find(m);
    if(it != ffters.end()) return *(it->second);
    return *(ffters[m] = new ConvolvePlanR2C(m));
}

//////////////////
//////////////////

Convolve_DCT_I::Convolve_DCT_I(unsigned int m): ConvolvePlanR2R(m) {
    d_fwd = fftw_plan_r2r_1d(M, realspace, kspace,
                             FFTW_REDFT00, FFTW_PATIENT);
    k_fwd = d_fwd;
    p_rev = fftw_plan_r2r_1d(M, kspace, realspace,
                             FFTW_REDFT00, FFTW_PATIENT);
}

ConvolvePlanR2R& Convolve_DCT_I::get_ffter(unsigned int m) {
    auto it = ffters.find(m);
    if(it != ffters.end()) return *(it->second);
    return *(ffters[m] = new Convolve_DCT_I(m));
}

map<unsigned int,ConvolvePlanR2R*> Convolve_DCT_I::ffters;

//////////////////
//////////////////

Convolve_DCT_DST_I::Convolve_DCT_DST_I(unsigned int m): ConvolvePlanR2R(m) {
    d_fwd = fftw_plan_r2r_1d(M, realspace, kspace,
                             FFTW_REDFT00, FFTW_PATIENT);
    k_fwd = fftw_plan_r2r_1d(M-2, realspace, kspace,
                             FFTW_RODFT00, FFTW_PATIENT);
    p_rev = fftw_plan_r2r_1d(M-2, kspace, realspace,
                             FFTW_RODFT00, FFTW_PATIENT);
}

ConvolvePlanR2R& Convolve_DCT_DST_I::get_ffter(unsigned int m) {
    auto it = ffters.find(m);
    if(it != ffters.end()) return *(it->second);
    return *(ffters[m] = new Convolve_DCT_DST_I(m));
}

map<unsigned int, ConvolvePlanR2R*> Convolve_DCT_DST_I::ffters;

///////////////////
///////////////////

Convolve_DCT_DST_II::Convolve_DCT_DST_II(unsigned int m): ConvolvePlanR2R(m) {
    d_fwd = fftw_plan_r2r_1d(M, realspace, kspace,
                             FFTW_REDFT10, FFTW_PATIENT);
    k_fwd = fftw_plan_r2r_1d(M, realspace, kspace,
                             FFTW_RODFT10, FFTW_PATIENT);
    p_rev = fftw_plan_r2r_1d(M-1, kspace, realspace,
                             FFTW_RODFT00, FFTW_PATIENT);
}

ConvolvePlanR2R& Convolve_DCT_DST_II::get_ffter(unsigned int m) {
    auto it = ffters.find(m);
    if(it != ffters.end()) return *(it->second);
    return *(ffters[m] = new Convolve_DCT_DST_II(m));
}

map<unsigned int, ConvolvePlanR2R*> Convolve_DCT_DST_II::ffters;

///////////////////
///////////////////

void ConvolverFactoryR2R::convolve(vector<double>& v) {
    auto& kern = getKernel(v.size()); // get kernel first, since migh change FFTer
    assert(kern.size() <= v.size());

    auto& ffter = getPlan(v.size());
    std::copy(v.begin(), v.end(), ffter.realspace);
    fftw_execute(ffter.d_fwd);
    ffter.kmul(kern);
    fftw_execute(ffter.p_rev);
    ffter.getResult(v);
}

const vector<double>& ConvolverFactoryR2R::getKernel(unsigned int i) {
    auto it = kdata.find(i);
    if(it != kdata.end()) return it->second;

    auto& ffter = getPlan(i);
    vector<double> kern = calcKernel(i);
    assert(kern.size() <= ffter.M);
    for(auto& k: kern) k /= ffter.normSize();

    std::fill(ffter.realspace, ffter.realspace+ffter.M, 0.);
    std::copy(kern.begin(), kern.end(), ffter.realspace);
    fftw_execute(ffter.k_fwd);

    auto& k = kdata[i];
    ffter.getkKern(k);
    return k;
}

/////////////////////
/////////////////////

vector<double> GaussConvolverFactory::calcKernel(unsigned int i) const {
    vector<double> v(i);
    double nrm = 0;
    for(int n=0; n<(int)i; n++) {
        v[n] = exp(-pow((n+0.5)/r,2)/2);
        nrm += (n? 2:1)*v[n];
    }
    for(auto& x: v) x /= nrm;
    return v;
}

///////////////////
///////////////////

void ConvolverFactoryR2C::convolve(vector<double>& v) {
    auto& kern = getKernel(v.size()); // get kernel first, since migh change FFTer

    auto& ffter = ConvolvePlanR2C::get_ffter(v.size());
    std::copy(v.begin(), v.end(), ffter.realspace);
    fftw_execute(ffter.d_fwd);
    for(size_t i=0; i<kern.size(); i++) ffter.kspace[i] *= kern[i];
    fftw_execute(ffter.p_rev);
    v.assign(ffter.realspace, ffter.realspace+v.size());
}

const vector<std::complex<double>>& ConvolverFactoryR2C::getKernel(unsigned int i) {
    auto it = kdata.find(i);
    if(it != kdata.end()) return it->second;

    auto& ffter = ConvolvePlanR2C::get_ffter(i);
    vector<double> kern = calcKernel(i);
    for(auto& k: kern) k /= ffter.normSize();

    std::copy(kern.begin(), kern.end(), ffter.realspace);
    fftw_execute(ffter.k_fwd);

    auto& v = kdata[i];
    v.assign(ffter.kspace, ffter.kspace+i/2+1);
    return v;
}
