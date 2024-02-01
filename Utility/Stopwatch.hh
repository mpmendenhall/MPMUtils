/// @file Stopwatch.hh Quickie profiling timer class
// -- Michael P. Mendenhall, 2019

#ifndef STOPWATCH_HH
#define STOPWATCH_HH

#include <string>
#include <chrono>
#include <stdio.h>
#include <stdexcept>

/// Stopwatch from construction to deletion
template<typename _clock_t = std::chrono::steady_clock>
class _Stopwatch {
public:
    /// clock to use
    typedef _clock_t clock_t;
    /// point in time
    typedef std::chrono::time_point<clock_t> timept_t;

    /// Constructor
    explicit _Stopwatch(bool go = true): running(go) { if(go) start();  }
    /// Destructor
    ~_Stopwatch() { if(running) stop(); }

    /// start counting
    void start() {
        if(running) throw std::logic_error("multiple stopwatch starts");
        //printf("Starting stopwatch...\n");
        running = true;
        t0 = now();
    }

    /// stop counting
    void stop() {
        if(!running) throw std::logic_error("stopwatch stop without start");
        running = false;
        elapsed += dtime();
        //if(elapsed == dt) printf("Elapsed time: %g seconds\n", elapsed);
        //else printf("Interval: %g, total elapsed %g seconds\n", dt, elapsed);
    }

    /// stop and restart
    void restart() { stop(); start(); }

    /// current elapsed time [s]
    double dtime() const { return dtime(t0, now()); }
    /// difference between timepoints
    static double dtime(timept_t _t0, timept_t _t1) { return std::chrono::duration<double>(_t1 - _t0).count(); }

    /// Get current elapsed; reset elapsed to 0
    double reset() {
        double e = elapsed;
        if(running) {
            auto t1 = now();
            e += dtime(t0, t1);
            t0 = t1;
        }
        elapsed = 0;
        return e;
    }

    /// get time now
    static timept_t now() { return clock_t::now(); }
    /// timept_t to epoch timestamp
    static double tstamp(timept_t t) {
        static double depoch = (
            std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count()
            - std::chrono::duration<double>(now().time_since_epoch()).count()
        );
        return depoch + std::chrono::duration<double>(t.time_since_epoch()).count();
    }
    /// timestamp now
    static double tstamp() { return tstamp(now()); }

    timept_t t0;            ///< starting time
    double elapsed = 0;     ///< total elapsed time

protected:
    bool running;
};

/// Default stopwatch type
typedef _Stopwatch<> Stopwatch;

#endif
