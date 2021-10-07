/// \file _DataSink.hh Non-typed generic bases
// -- Michael P. Mendenhall, LLNL 2021

#ifndef _DATASINK_HH
#define _DATASINK_HH

#include <stdexcept>
#include <typeinfo>
#include "_AnaIndex.hh"

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

/// Base marker for dynamic casting
class _DataSink: public SignalSink { };

/// Base marker for dynamic casting
class _SinkUser {
public:
    /// get nextSink output
    virtual _DataSink* _getNext() = 0;
    /// set nextSink output (throw if wrong type)
    virtual void _setNext(_DataSink* n) = 0;

    /// traverse chain to get last connected output
    virtual _SinkUser* lastSink() {
        auto n = _getNext();
        if(!n) return this;
        auto nn = dynamic_cast<_SinkUser*>(n);
        if(!nn) throw std::runtime_error("Non-traversable sinks chain");
        return nn->lastSink();
    }

    /// get AnaIndex<T> fot ouput datasink T
    virtual const _AnaIndex& getSinkIdx() const = 0;

    bool ownsNext = true;   ///< responsible for deleting output?
};

/// attempt to find output lastSink from any input
template<typename T>
_SinkUser* _find_lastSink(T* s) {
    auto i = dynamic_cast<_SinkUser*>(s);
    if(!i) throw std::runtime_error("Invalid input chain top class");
    return i->lastSink();
}

/// Redirection to a subsidiary sink output
class _SubSinkUser: virtual public _SinkUser {
public:
    /// Constructor
    _SubSinkUser(_SinkUser* s = nullptr): subSinker(s) { }
    /// get nextSink output
    _DataSink* _getNext() override {
        if(!subSinker) throw std::logic_error("undefined subSinker");
        return subSinker->_getNext();
    }
    /// set nextSink output (throw if wrong type)
    void _setNext(_DataSink* n) override {
        if(!subSinker) throw std::logic_error("undefined subSinker");
        subSinker->_setNext(n);
    }
    /// traverse chain to get last connected output
    _SinkUser* lastSink() override {
        if(!subSinker) throw std::logic_error("undefined subSinker");
        return subSinker->lastSink();
    }
    /// get AnaIndex<T> fot ouput datasink T
    const _AnaIndex& getSinkIdx() const override {
        if(!subSinker) throw std::logic_error("undefined subSinker");
        return subSinker->getSinkIdx();
    }

protected:
    _SinkUser* subSinker; ///< where to find output SinkUser
};

#endif
