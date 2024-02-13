/// @file

#include "Upsampler.hh"
#include <gsl/gsl_sf_trig.h>

void Upsampler::upsample(const vector<double>& vin, vector<double>& vout) {
    if(n_up == 1) { vout = vin; return; }
    if(!n_up) { vout.clear(); return; }

    auto n0 = vin.size();
    if(vin.data() == vout.data()) {
        const vector<double> v2 = vin;
        vout.clear();
        vout.resize(n0 * n_up);
        auto it = vout.begin();
        for(auto s: v2) { *it = s; it += n_up; }
    } else {
        vout.clear();
        vout.resize(vin.size() * n_up);
        auto it = vout.begin();
        for(auto s: vin) { *it = s; it += n_up; }
    }

    convolve(vout, vout);
    auto dn = kernsize()/2;
    vout.assign(vout.begin() + dn, vout.begin() + dn + n0*n_up);
}

void Upsampler::normalize_kernel(vector<double>& k) const {
    vector<double> n(n_up);
    size_t i = 0;
    for(auto x: k) n[(i++) % n_up] += x;
    i = 0;
    for(auto& x: k) x /= n[(i++) % n_up];
}

void Upsampler::set_sinc_interpolator(size_t nup, int nlobes, double sigma) {
    n_up = nup;
    if(!n_up) return;

    int w = 2 * n_up * nlobes;
    vector<double> v(w);
    double s2 = 2*sigma*sigma;

    for(int i = 0; i < w; ++i) {
        double x = double(i - w/2)/n_up;
        v[i] = gsl_sf_sinc(x) * exp(-x*x/s2);
    }

    normalize_kernel(v);
    setKernel(v);
}
