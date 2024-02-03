/// @file

#include "Boop.hh"

double Boop::envelope(double t) const {
    switch(eshape) {
        case FLAT: return 1;
        case TRIANGLE: return t< l*rise? t/(l*rise) : (l-t)/(l*(1-rise));
    }
    return 1;
}

float Boop::wave(double theta) const {
    if(timbre == 1) return sin(theta);
    return 2*atan(timbre*tan(sin(theta)*M_PI/2))/M_PI;
}

void Boop::gen(float* v, size_t stride, double dt) const {
    for(size_t i = 0; i < nsamps(); ++i) {
        double t = _t(i) + dt;
        *v += a * envelope(t) * wave(t*2*M_PI*f);
        v += stride;
    }
}
