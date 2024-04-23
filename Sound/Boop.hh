/// @file Boop.hh Sound blip waveform generator
// Michael P. Mendenhall

#ifndef BOOP_HH
#define BOOP_HH

#include <stdlib.h>
#include <cmath>
#include <vector>
using std::vector;

/// Base spec for a note
struct BoopSpec_t {
    double f = 500;     ///< base frequency
    double a = 1;       ///< amplitude scaling
    double l = 0.2;     ///< duration [s]
    double timbre = 1;  ///< adjustable waveform parameter
    double t0 = 0;      ///< start time
    int chan = 0;       ///< channel identifier
};

/// Base class boop synthesizer
class BoopSynth: public BoopSpec_t {
public:
    /// Constructor
    explicit BoopSynth(long srate = 48000, size_t nc = 2):
    samplerate(srate), nchan(nc) { }
    /// Destructor
    virtual ~BoopSynth() { }

    long samplerate;    ///< sample rate, Hz
    size_t nchan;       ///< number of channels

     /// number of samples that will be generated
    size_t nsamps() const { return l * samplerate; }

    /// time (s) for sample number i
    double _t(size_t i) const { return double(i)/samplerate; }
    /// time (s) to sample number and dt residual
    size_t samplepos(double& t) const { size_t i = samplerate*t; t -= i/double(samplerate); return i; }

    /// generate with automatic allocation
    virtual void gen(vector<float>& v, size_t i0 = 0) const;

protected:
    /// waveform at time t
    virtual float waveform(double /* t */) const { return 0; }
};

/// Simple boop synthesizer
class SimpleBoop: public BoopSynth {
public:
    /// Inherit Constructor
    using BoopSynth::BoopSynth;

    double rise = 0.2;  ///< envelope rise (envelope dependent)
    double efall = 0;   ///< tail falloff
    double chirp = 1;   ///< chirp frequency multiplier at end of sound

    /// envelope shape setting
    enum envelope_t {
        FLAT,       ///< constant step function
        TRIANGLE    ///< linear rise and fall
    } eshape = TRIANGLE;    ///< envelope shape

protected:
    /// amplitude envelope function
    double envelope(double t) const;

    /// oscillator waveform (2*pi periodic, aplitude +-1)
    float wave(double theta) const;

    /// waveform at time t
    float waveform(double t) const override;
};

#endif
