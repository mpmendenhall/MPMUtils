/// @file testSound.cc Test generating/playing sound
// Michael P. Mendenhall, 2024

#include "Boop.hh"
#include "WAVgen.hh"
#include "ConfigFactory.hh"

REGISTER_EXECLET(testSound) {
    WAVgen<> WG;
    WG.verbose = 5;
    Boop B(WG.samplerate);
    B.l = 0.15;

    WG.open_handle();
    WG.launch_mythread();

    for(size_t j = 0; j < 8; j++) {

        vector<float> buf(WG.nchan * B.nsamps());
        B.timbre = pow(1.2, j);
        B.f = 500*B.timbre;
        for(size_t i = 0; i < WG.nchan; ++i) {
            B.gen(buf.data() + i, WG.nchan);
            B.f *= 1.5;
        }

        WG.mapwrite(buf);
        WG.silence(0.2);
    }
}
