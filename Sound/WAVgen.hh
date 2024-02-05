/// @file WAVgen.hh ALSA-compatible .wav generator/player
// Michael P. Mendenhall, 2024

/*
may need to install: alsa-lib-devel
*/

#ifndef WAVEGEN_HH
#define WAVEGEN_HH

#include "PingpongBufferWorker.hh"
#include "BinaryIO.hh"
#include <string>
#include <random>
using std::string;

#ifdef WITH_ALSA
#include <alsa/asoundlib.h>
#endif

/// ALSA-compatible .wav format generator
template<typename _sample_t = int16_t>
class WAVgen: public PingpongBufferWorker<const vector<_sample_t>> {
public:
    typedef _sample_t sample_t;
    typedef PingpongBufferWorker<const vector<sample_t>> PBW;
    static constexpr size_t bytes_per_sample = sizeof(sample_t); ///< storage size of one sample
    static constexpr double min_a = std::numeric_limits<sample_t>::min(); ///< amplitude min for format
    static constexpr double max_a = std::numeric_limits<sample_t>::max(); ///< amplitude max for format

    /// Constructor
    WAVgen(): dith(-0.5, 0.5) { }
    /// Destructor
    virtual ~WAVgen() { close(); }

    long samplerate = 48000;    ///< sample rate, Hz
    double latency = 0.5;       ///< buffer latency, s
    size_t nchan = 2;           ///< number of channels
    BinaryWriter* fOut = nullptr;   ///< output .wav file

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

    /// initialize headers in output file
    void initOut(BinaryWriter& B) {
        fOut = &B;
        WriteScope _B(B);
        B << 'R' << 'I' << 'F' << 'F';
        B << int32_t(36 + 0x0FFFFF00);   //< update later with file size
        B << 'W' << 'A' << 'V' << 'E';
        B << 'f' << 'm' << 't' << ' ';
        B << int32_t(8*sizeof(sample_t));
        B << int16_t(1) /* uncompressed PCM */ << int16_t(nchan);
        B << int32_t(samplerate);
        B << int32_t(samplerate * nchan * sizeof(sample_t));
        B << int16_t(nchan * sizeof(sample_t)) << int16_t(8*sizeof(sample_t));
        B << 'd' << 'a' << 't' << 'a';
        B << int32_t(0xFFFFFFFF); //< update later with data size
    }

    /// send t seconds "silence" data to output buffer (blocking)
    void silence(double t) {
        if(!(t > 0)) return;
        static vector<sample_t> v;
        v.clear();
        v.resize(nchan * size_t(samplerate * t));
        for(auto& s: v) s = normalize(0.); // dither and baseline
        PBW::add_item(v);
    }

    /// map (-1,1) float to internal format range, with dithering
    sample_t normalize(float in) {
        in += dith(gen)/(max_a - min_a);
        in = std::max(std::min(in, float(1.)), float(-1.));
        return std::round(-0.5*min_a*(in - 1) + max_a*0.5*(in + 1));
    }

    /// remap float to native type/range and write
    void mapwrite(const vector<float>& b) {
        static vector<sample_t> v;
        v.resize(b.size());
        auto it = v.begin();
        for(auto s: b) *(it++) = normalize(s);
        PBW::add_item(v);
    }

    /// finish processing and close connection (automatic on destruct)
    void close() {
        if(PBW::checkRunning()) PBW::finish_mythread();
#ifdef WITH_ALSA
        if(handle) snd_pcm_close(handle);
        handle = nullptr;
#endif
    }

protected:
    std::mt19937 gen;
    std::uniform_real_distribution<> dith;

    /// send data to output buffer (blocking)
    void write(const vector<sample_t>& dat) {
        if(!dat.size()) return;
        if(fOut) fOut->sendblock(dat);
        sprev = dat.back();
#ifdef WITH_ALSA
        if(!handle) return;
        long nframes = dat.size()/nchan;
        auto frames = snd_pcm_writei(handle, dat.data(), nframes);
        if(frames < 0) frames = snd_pcm_recover(handle, frames, 0 /* attempt silent error recovery */);
        if(frames < 0) printf("snd_pcm_writei failed: %s\n", snd_strerror(frames));
        if(frames > 0 && frames < nframes) printf("Short write (expected %li, wrote %li)\n", nframes, frames);
#endif
    }

    /// play buffered items
    void processout() override {
        PBW::processout();
        for(auto& v: PBW::_datq) write(v);
    }

#ifdef WITH_ALSA
    snd_pcm_t* handle = nullptr;                  ///< output identifier
    snd_pcm_format_t fmt = SND_PCM_FORMAT_S16_LE; ///< output data format
#endif

};

#endif
