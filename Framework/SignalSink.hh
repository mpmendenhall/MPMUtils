/// \file SignalSink.hh Receiver for datastream signals
// -- Michael P. Mendenhall, LLNL 2022

#ifndef SIGNALSINK_HH
#define SIGNALSINK_HH

#include "to_str.hh"

/// Data processing signals side-channel info
enum datastream_signal_t {
    DATASTREAM_NOOP     = 0,    ///< ignore me!
    DATASTREAM_INIT     = 1,    ///< once-per-analysis initialization
    DATASTREAM_START    = 2,    ///< start of data block
    DATASTREAM_CHECKPT  = 3,    ///< mid-calculation "checkpoint" request

    DATASTREAM_FLUSH    = 99994,///< "breakpoint" data flush
    DATASTREAM_REINIT   = 99995,///< initialize for new upstream source
    DATASTREAM_END      = 99996 ///< once-per-analysis end of data
};

/// Printable name for signals
inline string signal_name(datastream_signal_t s) {
    switch(s) {
        case DATASTREAM_NOOP:   return "No-Op";
        case DATASTREAM_INIT:   return "Init";
        case DATASTREAM_START:  return "Start";
        case DATASTREAM_CHECKPT:return "Checkpoint";
        case DATASTREAM_FLUSH:  return "Flush";
        case DATASTREAM_REINIT: return "ReInit";
        case DATASTREAM_END:    return "End";
        default: return to_str(s);
    }
}

/// Base class passing signals
class SignalSink {
public:
    /// Polymorphic Destructor
    virtual ~SignalSink() { }
    /// accept data flow signal
    virtual void signal(datastream_signal_t) { }
};

#endif
