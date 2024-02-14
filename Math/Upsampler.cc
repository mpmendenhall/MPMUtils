/// @file

#include "Upsampler.hh"
#include <gsl/gsl_sf_trig.h>

void Upsampler::upsample(const vector<double>& _vin, vector<double>& vout) {
    if(n_up == 1) { vout = _vin; return; }
    if(!n_up) { vout.clear(); return; }

    // pad out end for full chunked calculation
    auto n = N/n_up;
    vector<double> v_in(n);
    prepoints(_vin, v_in);
    v_in.insert(v_in.end(), _vin.begin(), _vin.end());
    postpoints(_vin, v_in, n-1);

    // pre-upsampling points lattice
    auto n0 = v_in.size();
    vector<double> vs(n_up * n0);
    auto it = vs.begin();
    for(size_t i = 0; i < n0; ++i) {
        *it = v_in[i];
        it += n_up;
    }

    n0 = _vin.size();
    _convolve(vs, vout, n0 * n_up);
    auto dn = kernsize()/2;
    vout.assign(vout.begin() + dn, vout.begin() + dn + n0 * n_up);
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
