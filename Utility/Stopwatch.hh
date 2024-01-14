/// @file Stopwatch.hh Quickie profiling timer class
// -- Michael P. Mendenhall, 2019

#ifndef STOPWATCH_HH
#define STOPWATCH_HH

#include <string>
#include <chrono>
#include <stdio.h>

/// Stopwatch from construction to deletion
class Stopwatch {
public:
    /// Constructor
    Stopwatch(bool go = true): running(!go) { if(go) start();  }
    /// Destructor
    ~Stopwatch() { if(running) stop(); }

    /// start counting
    void start() {
        if(running) throw;
        printf("Starting stopwatch...\n");
        running = true;
        t0 = std::chrono::steady_clock::now();
    }

    /// stop counting
    void stop() {
        if(!running) throw;
        running = false;
        auto t1 = std::chrono::steady_clock::now();
        auto dt = std::chrono::duration<double>(t1-t0).count();
        elapsed += dt;
        if(elapsed == dt) printf("Elapsed time: %g seconds\n", elapsed);
        else printf("Interval: %g, total elapsed %g seconds\n", dt, elapsed);
    }

protected:
    bool running;
    std::chrono::time_point<std::chrono::steady_clock> t0;  ///< starting time
    double elapsed = 0;                                     ///< total elapsed time
};

#endif
