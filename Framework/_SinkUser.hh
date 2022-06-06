/// \file _SinkUser.hh Non-typed generic bases
// -- Michael P. Mendenhall, LLNL 2021

#ifndef _SINKUSER_HH
#define _SINKUSER_HH

#include "SignalSink.hh"
#include <stdexcept>
#include <typeinfo>
#include "ConfigFactory.hh"

class _AnaIndex;

/// Base marker for dynamic casting
class _SinkUser {
public:
    /// get nextSink output
    virtual SignalSink* _getNext() { return nullptr; }
    /// set nextSink output (throw if wrong type)
    virtual void _setNext(SignalSink*) { throw std::logic_error("Need specific data type to _setNext"); }
    /// set ownership of nextSink
    virtual void setOwnsNext(bool) { throw std::logic_error("Need specific data type to setOwnsNext"); }

    /// traverse chain to get last connected output
    virtual _SinkUser* lastSink() {
        auto n = _getNext();
        if(!n) return this;
        auto nn = dynamic_cast<_SinkUser*>(n);
        if(!nn) throw std::runtime_error("Non-traversable sinks chain");
        return nn->lastSink();
    }

    /// signal "nextSink"
    virtual void sigNext(datastream_signal_t s) {
        auto n = _getNext();
        if(n) n->signal(s);
    }

    /// get AnaIndex<T> fot ouput datasink T
    virtual const _AnaIndex& getSinkIdx() const;

    /// generate appropriat configured data sink type
    virtual SignalSink* makeDataSink(const Setting&, const string& = "") const { return new SignalSink(); }

    /// construct and attach configured output sink
    virtual void createOutput(const Setting& S, const string& dfltclass = "") {
        _setNext(makeDataSink(S, dfltclass));
    }
};

/// find output lastSink from any input
template<typename T>
_SinkUser* _find_lastSink(T* s, bool can_return_nullptr = false) {
    auto i = dynamic_cast<_SinkUser*>(s);
    if(!i) {
        if(can_return_nullptr) return nullptr;
        throw std::runtime_error("Invalid input chain top class");
    }
    try { return i->lastSink(); }
    catch(...) {
        if(!can_return_nullptr) throw;
        return nullptr;
    }
}

/// Redirection to a subsidiary sink output
class _SubSinkUser: public _SinkUser {
public:
    /// Constructor
    _SubSinkUser(_SinkUser* s = nullptr): subSinker(s) { }
    /// get nextSink output
    SignalSink* _getNext() override {
        if(!subSinker) throw std::logic_error("undefined subSinker");
        return subSinker->_getNext();
    }
    /// set nextSink output (throw if wrong type)
    void _setNext(SignalSink* n) override {
        if(!subSinker) throw std::logic_error("undefined subSinker");
        subSinker->_setNext(n);
    }
    /// traverse chain to get last connected output
    _SinkUser* lastSink() override {
        if(!subSinker) throw std::logic_error("undefined subSinker");
        auto o = subSinker->lastSink();
        return o == subSinker? this : o;
    }
    /// get AnaIndex<T> fot ouput datasink T
    const _AnaIndex& getSinkIdx() const override {
        if(!subSinker) throw std::logic_error("undefined subSinker");
        return subSinker->getSinkIdx();
    }
    /// set ownership of nextSink
    void setOwnsNext(bool b) override {
        if(!subSinker) throw std::logic_error("undefined subSinker");
        subSinker->setOwnsNext(b);
    }

protected:
    _SinkUser* subSinker; ///< where to find output SinkUser
};

#include "_AnaIndex.hh"
/// get AnaIndex<T> fot ouput datasink T
inline const _AnaIndex& _SinkUser::getSinkIdx() const { static _AnaIndex I; return I; }

#endif
