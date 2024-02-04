/// @file MicSim.hh Microphone placement simulation

#ifndef MICSIM_HH
#define MICSIM_HH

#include "Vec.hh"
#include <vector>
using std::vector;
#include <utility>
using std::pair;

/// Microphone info
class MicPlacement {
public:
    /// 3-vector datatype
    typedef Vec<3, double> vec3_t;
    /// speed of sound [m/s]
    static constexpr double v_snd = 343.;

    vec3_t pos{};           ///< position
    vec3_t dir{{0.,0.,1.}}; ///< orientation unit vector

    /// pickup pattern
    enum pickup_t {
        OMNI,       ///< omnidirectional
        CARDIOID,   ///< cardiod
        FIGURE8     ///< figure 8
    } pickup;       ///< pickup pattern

    /// calculated pickup response
    struct response_t {
        double dt = 0;  ///< timing offset [s]
        double a = 1;   ///< normalized amplitude [m^2]
    };

    /// Constructor
    explicit MicPlacement(pickup_t p = OMNI): pickup(p) { }

    /// response to point source at x
    response_t response(vec3_t x) const;
    /// print info to stdout
    void display() const;
};

/// "Mixer" for multiple mic placements
class MicSim {
public:
    /// reuse vec3_t
    typedef MicPlacement::vec3_t vec3_t;
    /// mixture from indexed mics
    typedef vector<pair<size_t, double>> mix_t;
    /// response for each item in mix_t
    typedef vector<MicPlacement::response_t> mix_response_t;

    /// response from each of a selected set of microphones
    mix_response_t mix_response(const mix_t& m, vec3_t x) const;
    /// response for enumerated channel
    mix_response_t mix_response(size_t chan, vec3_t x) const { return mix_response(chans.at(chan), x); }

    vector<MicPlacement> mics;  ///< enumerated microphones set
    vector<mix_t> chans;        ///< channel mixes

    //---------------------------
    // conventions:
    //  microphones spaced along x axis,
    //  pointing towards "depth" z; y is "up".
    //  channel order is L,R = -x, +x

    /// new channel pair from spaced omnis
    void setup_spaced_omni(double l = 1.);
    /// new channel pair from spaced angled cardiods with opening angle (degrees)
    void setup_cardiod_pair(double aopen = 135., double l = 0.1);

    /// create single-microphone channel for microphone i
    void make_mic_chan(size_t i, double a = 1.) {  chans.emplace_back(); chans.back().emplace_back(i, a); }
    /// create new default mic with channel
    size_t new_mic_chan(MicPlacement::pickup_t p = MicPlacement::OMNI, double a = 1.);

    /// print info to stdout
    void display() const;
};

#endif
