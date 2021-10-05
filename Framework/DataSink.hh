/// \file DataSink.hh Base class for accepting a stream of objects
// -- Michael P. Mendenhall, LLNL 2019

#ifndef DATASINK_HH
#define DATASINK_HH

#include <stdexcept>

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

/// Base class passing signals
class SignalSink {
public:
    /// Polymorphic Destructor
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

/// Base marker for dynamic casting
class _SinkUser {
public:
    /// get nextSink output (throw if impossible to cast)
    virtual _SinkUser* _getNext() = 0;

    /// traverse chain to get last connected output
    _SinkUser* lastSink() {
        auto n = _getNext();
        return n? n->lastSink() : this;
    }
};

/// Base class outputting to a sink
template<typename T>
class SinkUser: public _SinkUser {
public:
    DataSink<T>* nextSink = nullptr;    ///< recipient of output

    /// Destructor
    ~SinkUser() { delete nextSink; }

    /// get nextSink as SinkUser... if possible
    _SinkUser* _getNext() override {
        if(!nextSink) return nullptr;
        auto n = dynamic_cast<_SinkUser*>(nextSink);
        if(!n) throw std::runtime_error("Non-traversable sinks chain");
        return n;
    }
};

/// Combined input/output link in analysis chain
template<typename T, typename U>
class DataLink: public DataSink<T>, public SinkUser<U> {
public:
    /// pass through data flow signal
    void signal(datastream_signal_t s) override {
        if(this->nextSink) this->nextSink->signal(s);
    }
};

#endif
