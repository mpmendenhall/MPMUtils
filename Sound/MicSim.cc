/// @file

#include "MicSim.hh"
#include <iostream>
using std::cout;

MicPlacement::response_t MicPlacement::response(vec3_t x) const {
    auto dx = x - pos;
    auto r2 = dx.mag2();
    auto a = 1./r2;
    auto r = sqrt(r2);

    if(pickup != OMNI) {
        auto k = dir.dot(dx)/r;
        if(pickup == FIGURE8) a *= k;
        if(pickup == CARDIOID) a *= 0.5*(k + 1);
    }

    return response_t(r/v_snd, a);
}

void MicPlacement::display() const {
    if(pickup == OMNI) cout << "Omni";
    else if(pickup == CARDIOID) cout << "Cardioid";
    else if(pickup == FIGURE8) cout << "Figure-8";
    else cout << "Microphone";

    cout << " at " << pos << " towards " << dir << std::endl;
}

//----------------------------------------

MicSim::mix_response_t MicSim::mix_response(const mix_t& m, vec3_t x) const {
    mix_response_t v(m.size());
    auto it = v.begin();
    for(auto& k: m) {
        *it = mics.at(k.first).response(x);
        it->a *= k.second;
        ++it;
    }
    return v;
}

size_t MicSim::new_mic_chan(MicPlacement::pickup_t p, double a) {
    size_t i = mics.size();
    mics.emplace_back(p);
    make_mic_chan(i,a);
    return i;
}

void MicSim::setup_spaced_omni(double l) {
    mics[new_mic_chan()].pos[0] = -l/2;
    mics[new_mic_chan()].pos[0] = l/2;
}

void MicSim::setup_cardiod_pair(double aopen, double l) {
    aopen *= 0.5*M_PI/180;
    vec3_t d{{-sin(aopen), 0., cos(aopen)}};

    auto& m1 = mics[new_mic_chan(MicPlacement::CARDIOID)];
    m1.pos[0] = -l/2;
    m1.dir = d;

    auto& m2 = mics[new_mic_chan(MicPlacement::CARDIOID)];
    m2.pos[0] = l/2;
    d[0] = -d[0];
    m2.dir = d;
}

void MicSim::display() const {
    cout << "MicSim with microphones:" << std::endl;
    size_t i = 0;
    for(auto& m: mics) {
        cout << " [" << i++ << "] ";
        m.display();
    }
    cout << "and channel mixes:" << std::endl;
    i = 0;
    for(auto& c: chans) {
        cout << " (" << i++ << ")";
        for(auto& p: c) cout << " " << p.second << " * " << "[" << p.first << "]";
        cout << std::endl;
    }
}
