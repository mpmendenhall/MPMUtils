/// @file Boop.hh Sound blip waveform generator
// Michael P. Mendenhall

#ifndef BOOP_HH
#define BOOP_HH

#include <stdlib.h>
#include <cmath>
#include <vector>
using std::vector;

/// Sound blip synth
class Boop {
public:
    /// Constructor
    Boop(long srate = 48000, size_t nc = 2):
    samplerate(srate), nchan(nc) { }

    long samplerate;    ///< sample rate, Hz
    size_t nchan;       ///< number of channels

    double f = 500;     ///< base frequency
    double a = 1;       ///< amplitude scaling
    double l = 0.2;     ///< duration [s]
    double rise = 0.2;  ///< envelope rise (envelope dependent)
    double timbre = 1;  ///< waveform timbre adjust
    double efall = 1;   ///< tail falloff

    /// number of samples that will be generated
    size_t nsamps() const { return l * samplerate; }

    /// time (s) for sample number i
    double _t(size_t i) const { return double(i)/samplerate; }
    /// time (s) to sample number and dt residual
    size_t samplepos(double& t) const { size_t i = samplerate*t; t -= i/double(samplerate); return i; }

    /// envelope shape setting
    enum envelope_t {
        FLAT,       ///< constant step function
        TRIANGLE    ///< linear rise and fall
    } eshape = TRIANGLE;    ///< envelope shape

    /// amplitude envelope function
    double envelope(double t) const;

    /// oscillator waveform (2*pi periodic, aplitude +-1)
    float wave(double theta) const;

    /// generate and add sound to buffer
    void gen(float* v, size_t stride = 1, double dt = 0) const;
    /// generate with automatic allocation
    void gen(vector<float>& v, double t0, int chan) const;
};

#endif
