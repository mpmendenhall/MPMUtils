/// @file WAVgen.hh ALSA-compatible .wav generator/player
// Michael P. Mendenhall, 2024

#ifndef WAVEGEN_HH
#define WAVEGEN_HH

#include "PingpongBufferWorker.hh"
#include <string>
using std::string;

#ifdef WITH_ALSA
#include <alsa/asoundlib.h>
#endif

/// ALSA-compatible .wav format generator
template<typename _sample_t = unsigned char>
class WAVgen: public PingpongBufferWorker<const vector<_sample_t>> {
public:
    typedef _sample_t sample_t;
    typedef PingpongBufferWorker<const vector<sample_t>> PBW;

    /// Destructor
    virtual ~WAVgen() { close(); }

    long samplerate = 48000;    ///< sample rate, Hz
    double latency = 0.5;       ///< buffer latency, s
    size_t nchan = 2;           ///< number of channels

    size_t bytes_per_sample = 1;        ///< storage size of one sample
    double min_a = 0;                   ///< amplitude min for format
    double max_a = 255;                 ///< amplitude max for format
    sample_t sprev = (min_a + max_a)/2; ///< end of previous sample sequence

    /// initialize sound output handle
    void open_handle() {
#ifdef WITH_ALSA
        int err =  snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
        if(err < 0) throw std::runtime_error("Playback open error: " + string(snd_strerror(err)));
        err = snd_pcm_set_params(handle, fmt, SND_PCM_ACCESS_RW_INTERLEAVED,
                                 nchan, samplerate, 1 /* allow resampling */, latency * 1e6);
        if(err < 0) throw std::runtime_error("Playback open error: " + string(snd_strerror(err)));
#endif
    }

    /// send t seconds "silence" data to output buffer (blocking)
    void silence(double t) {
        static vector<sample_t> v;
        v.resize(nchan * samplerate * t, (min_a + max_a)/2);
        PBW::add_item(v);
    }

    /// map (-1,1) float to internal format range
    template<typename S>
    void normalize(float in, S& out) const {
        in = std::max(std::min(in, float(1.)), float(-1.));
        out = -0.5*min_a*(in - 1) + max_a*0.5*(in + 1);
    }

    /// remap float to native type/range and write
    void mapwrite(const vector<float>& b) {
        static vector<sample_t> v;
        v.resize(b.size());
        auto it = v.begin();
        for(auto s: b) normalize(s, *(it++));
        PBW::add_item(v);
    }

    /// finish processing and close connection (automatic on destruct)
    void close() {
        if(PBW::checkRunning()) PBW::finish_mythread();
        if(handle) snd_pcm_close(handle);
        handle = nullptr;
    }

protected:
    /// send data to output buffer (blocking)
    void write(const vector<sample_t>& dat) {
        if(!dat.size()) return;
#ifdef WITH_ALSA
        if(!handle) throw std::logic_error("Output handle uninitialized");
        long nframes = dat.size()/nchan;
        auto frames = snd_pcm_writei(handle, dat.data(), nframes);
        if(frames < 0) frames = snd_pcm_recover(handle, frames, 0 /* attempt silent error recovery */);
        if(frames < 0) printf("snd_pcm_writei failed: %s\n", snd_strerror(frames));
        if(frames > 0 && frames < nframes) printf("Short write (expected %li, wrote %li)\n", nframes, frames);
#endif
        sprev = dat.back();
    }

    /// play buffered items
    void processout() override {
        PBW::processout();
        for(auto& v: PBW::_datq) write(v);
    }

#ifdef WITH_ALSA
    snd_pcm_t* handle = nullptr;                ///< output identifier
    snd_pcm_format_t fmt = SND_PCM_FORMAT_U8;   ///< output data format
#endif

};

#endif
