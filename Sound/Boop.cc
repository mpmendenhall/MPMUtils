/// @file

#include "Boop.hh"

double Boop::envelope(double t) const {
    if(t <= 0) return 0;
    if(eshape == FLAT) return 1;
    if(eshape == TRIANGLE) {
        if(t < l*rise) return t/(l*rise);
        double x = (l-t)/(l*(1-rise));
        return x * exp(-efall*(1-x));
    }
    return 1;
}

float Boop::wave(double theta) const {
    if(timbre == 1) return sin(theta);
    return 2*atan(timbre*tan(sin(theta)*M_PI/2))/M_PI;
}

void Boop::gen(float* v, size_t stride, double dt) const {
    for(size_t i = 0; i < nsamps(); ++i) {
        double t = _t(i) - dt;
        *v += a * envelope(t) * wave(t*2*M_PI*f);
        v += stride;
    }
}

void Boop::gen(vector<float>& v, double t0, int chan, size_t i0) const {
    auto i = samplepos(t0);
    auto sz = nchan*(i + nsamps()) + i0;
    if(sz > v.size()) v.resize(sz);
    gen(v.data() + i*nchan + chan + i0, nchan, t0);
}
