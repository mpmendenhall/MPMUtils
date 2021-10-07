/// \file DataSink.hh Base class for accepting a stream of objects
// -- Michael P. Mendenhall, LLNL 2019

#ifndef DATASINK_HH
#define DATASINK_HH

#include "_DataSink.hh"
#include "AnaIndex.hh"

/// Virtual base class for accepting a stream of objects
template<typename T>
class DataSink: public _DataSink {
public:
    typedef T sink_t;

    /// take instance of object
    virtual void push(sink_t&) = 0;
};

/// Base class outputting to a sink
template<typename T>
class SinkUser: virtual public _SinkUser {
public:
    /// output data type
    typedef T output_t;
    /// output sink type
    typedef DataSink<output_t> dsink_t;

    /// Destructor
    ~SinkUser() { if(ownsNext) delete nextSink; }

    /// get nextSink
    _DataSink* _getNext() override { return getNext(); }

    /// set nextSink output (throw if wrong type)
    void _setNext(_DataSink* n) override {
        auto& nxt = getNext();
        if(!n) nxt = nullptr;
        else {
            auto nn = dynamic_cast<dsink_t*>(n);
            if(!nn) throw std::logic_error("incompatible nextSink assignment");
            nxt = nn;
        }
    }

    /// get assignable nextSink
    dsink_t*& getNext() { return nextSink; }

    /// get AnaIndex<T> fot ouput datasink T
    const _AnaIndex& getSinkIdx() const override {
        static AnaIndex<T> I;
        return I;
    }

    /// pass through data flow signal
    virtual void su_signal(datastream_signal_t s) { if(nextSink) nextSink->signal(s); }

protected:
    dsink_t* nextSink = nullptr;    ///< recipient of output
};

/// attempt to find output lastSink from any input
template<typename T, typename U>
SinkUser<T>* find_lastSinkUser(U* s) {
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
        PreTransform.getNext() = &myXfer;
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

#include "ConfigCollator.hh"

#endif
