/// \file DataSink.hh Base class for accepting a stream of objects
// -- Michael P. Mendenhall, LLNL 2019

#ifndef DATASINK_HH
#define DATASINK_HH

#include <stdexcept>
#include <typeinfo>

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
    virtual _SinkUser* lastSink() {
        auto n = _getNext();
        return n? n->lastSink() : this;
    }
};

/// attempt to find output lastSink from any input
template<typename T>
_SinkUser* _find_lastSink(T* s) {
    auto i = dynamic_cast<_SinkUser*>(s);
    if(!i) throw std::runtime_error("Invalid input chain top class");
    return i->lastSink();
}

/// Redirection to a subsidiary sink output
class SubSinkUser: public _SinkUser {
public:
    /// Constructor
    SubSinkUser(_SinkUser* s = nullptr): subSinker(s) { }
    /// get nextSink output
    _SinkUser* _getNext() override { return subSinker? subSinker->_getNext() : nullptr; }
    /// traverse chain to get last connected output
    _SinkUser* lastSink() override {
        auto n = _getNext();
        return n? n->lastSink() : subSinker;
    }
protected:
    _SinkUser* subSinker; ///< where to find output SinkUser
};

/// Base class outputting to a sink
template<typename T>
class SinkUser: public _SinkUser {
public:
    /// output data type
    typedef T output_t;
    /// output sink type
    typedef DataSink<output_t> dsink_t;

    dsink_t* nextSink = nullptr;    ///< recipient of output
    bool ownsNext = true;           ///< responsible for deleting output?

    /// Destructor
    ~SinkUser() { if(ownsNext) delete nextSink; }

    /// get nextSink as SinkUser... if possible
    _SinkUser* _getNext() override {
        if(!nextSink) return nullptr;
        auto n = dynamic_cast<_SinkUser*>(nextSink);
        if(!n) throw std::runtime_error("Non-traversable sinks chain");
        return n;
    }

    /// pass through data flow signal
    virtual void su_signal(datastream_signal_t s) { if(nextSink) nextSink->signal(s); }
};

/// attempt to find output lastSink from any input
template<typename T, typename U>
SinkUser<T>* find_lastSink(U* s) {
    auto i = dynamic_cast<_SinkUser*>(s);
    if(!i) throw std::runtime_error("Invalid input chain top class");
    auto j = dynamic_cast<SinkUser<T>*>(i->lastSink());
    if(!j) throw std::runtime_error("Incorrect output type for input chain");
    return j;
}

/// Combined input/output link in analysis chain
template<typename T, typename U>
class DataLink: public DataSink<T>, public SinkUser<U> {
public:
    /// pass through data flow signal
    void signal(datastream_signal_t s) override { SinkUser<U>::su_signal(s); }
};


/// Hidden input transform stage helper (PT -> DataLink)
template<class PT>
class PreSink: public DataSink<typename PT::sink_t> {
public:
    typedef PT presink_t;
    typedef typename presink_t::sink_t input_t;
    typedef typename presink_t::output_t mid_t;

protected:
    presink_t PreTransform; ///< pre-transform stage

    /// data transfer helper
    class _xfer: public DataSink<mid_t> {
    public:
        _xfer(PreSink& _out): out(_out) { }
        void push(mid_t& o) override { out._push(o); }
        void signal(datastream_signal_t s) override { out._signal(s); }
    protected:
        PreSink& out;
    } myXfer;

public:

    /// pass-through constructor
    template<typename... Args>
    explicit PreSink(Args&&... a): PreTransform(std::forward<Args>(a)...), myXfer(*this) {
        PreTransform.nextSink = &myXfer;
        PreTransform.ownsNext = false;
    }

    /// pass input to pre-filter
    void push(input_t& o) override { PreTransform.push(o); }
    /// pass through signals
    void signal(datastream_signal_t s) override { PreTransform.signal(s); }

    /// Override to handle pre-transformed input:
    virtual void _push(mid_t&) { }
    /// Override to handle signals through pre-transform
    virtual void _signal(datastream_signal_t) { }
};

/// DataLink with Pre-Filter
template<class PT, class U>
class PSDataLink: public PreSink<PT>, public SinkUser<U> {
public:
    /// pass-through constructor
    template<typename... Args>
    explicit PSDataLink(Args&&... a): PreSink<PT>(std::forward<Args>(a)...) { }

    /// receive back signals
    void _signal(datastream_signal_t s) override { SinkUser<U>::su_signal(s); }
};

#endif
