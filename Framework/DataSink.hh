/// \file DataSink.hh Base class for accepting a stream of objects
// -- Michael P. Mendenhall, LLNL 2019

#ifndef DATASINK_HH
#define DATASINK_HH

/// Data processing signals side-channel info
enum datastream_signal_t {
    DATASTREAM_INIT     = 1,    ///< once-per-analysis initialization
    DATASTREAM_START    = 2,    ///< start of data block
    DATASTREAM_CHECKPT  = 3,    ///< mid-calculation "checkpoint" request

    DATASTREAM_FLUSH    = 4,    ///< "breakpoint" data flush
    DATASTREAM_REINIT   = 5,    ///< initialize for new upstream source
    DATASTREAM_END      = 6     ///< once-per-analysis end of data
};

/// Base class passing signals
class SignalSink {
public:
    /// Destructor
    virtual ~SignalSink() { }
    /// accept data flow signal
    virtual void signal(datastream_signal_t) { }
};

/// Virtual base class for accepting a stream of objects
template<typename T>
class DataSink: public SignalSink {
public:
    typedef T sink_t;

    /// take instance of object
    virtual void push(sink_t&) = 0;
};

#endif
