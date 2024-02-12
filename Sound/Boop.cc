/// @file

#include "Boop.hh"

void BoopSynth::gen(vector<float>& v, size_t i0) const {
    auto _t0 = t0;
    auto i = samplepos(_t0);
    auto sz = nchan*(i + nsamps()) + i0;
    if(sz > v.size()) v.resize(sz);

    auto it = v.begin() + i0 + chan;
    for(i = 0; i < nsamps(); ++i) {
        double t = _t(i) - _t0;
        *it += waveform(t);
        it += nchan;
    }
}

//-----------------------

double SimpleBoop::envelope(double t) const {
    if(t <= 0) return 0;
    if(eshape == FLAT) return 1;
    if(eshape == TRIANGLE) {
        if(t < l*rise) return t/(l*rise);
        double x = (l-t)/(l*(1-rise));
        return x * exp(-efall*(1-x));
    }
    return 1;
}

float SimpleBoop::wave(double theta) const {
    if(timbre == 1) return sin(theta);
    return 2*atan(timbre*tan(sin(theta)*M_PI/2))/M_PI;
}

float SimpleBoop::waveform(double t) const {
    double c = t / l;
    return a * envelope(t) * wave(t*2*M_PI*((1-c)*f + c*chirp*f));
}
