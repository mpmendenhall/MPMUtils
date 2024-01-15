/// @file SinkUser.hh Base classes using an output DataSink
// -- Michael P. Mendenhall, LLNL 2022

#ifndef SINKUSER_HH
#define SINKUSER_HH

#include "XMLTag.hh"
#include "_SinkUser.hh"
#include "DataSink.hh"

/// Base class outputting to a sink
template<typename T>
class SinkUser: virtual public _SinkUser {
public:
    /// output data type
    typedef T output_t;
    /// mutable variant of received data type
    typedef typename std::remove_const<output_t>::type outmut_t;
    /// output sink type
    typedef DataSink<output_t> dsink_t;

    /// Destructor
    ~SinkUser() { if(ownsNext) delete nextSink; }

    /// get nextSink
    SignalSink* _getNext() override { return getNext(); }
    /// set ownership of nextSink
    void setOwnsNext(bool b) override { ownsNext = b; }

    /// set output
    virtual void setNext(dsink_t* n) {
        auto& nxt = getNext();
        if(nxt) throw std::logic_error("nextSink already assigned");
        nxt = n;
        auto x = dynamic_cast<XMLProvider*>(this);
        if(x) x->tryAdd(nxt);
    }

    /// set nextSink output (throw if wrong type)
    void _setNext(SignalSink* n) override {
        if(!n) setNext(nullptr);
        else {
            auto nn = dynamic_cast<dsink_t*>(n);
            if(!nn) throw std::logic_error("incompatible nextSink assignment");
            setNext(nn);
        }
    }

    /// get assignable nextSink
    dsink_t*& getNext() { return nextSink; }

    /// get AnaIndex<T> fot ouput datasink T
    const _AnaIndex& getSinkIdx() const override;

    /// generate appropriat configured data sink type
    SignalSink* makeDataSink(const ConfigInfo_t& S, const string& dfltclass = "") const override {
        auto snk = constructCfgObj<DataSink<T>>(S, dfltclass);
        if(snk) snk->initialize();
        return snk;
    }

    /// pass through data flow signal
    virtual void su_signal(datastream_signal_t s) { if(nextSink) nextSink->signal(s); }

protected:
    bool ownsNext = true;           ///< responsible for deleting output?
    dsink_t* nextSink = nullptr;    ///< recipient of output
};

/// attempt to find output lastSink from any input
template<typename T>
_SinkUser* find_lastSink(T* s) {
    auto i = dynamic_cast<_SinkUser*>(s);
    if(!i) throw std::runtime_error("Invalid input chain top class");
    return i->lastSink();
}

/// Combined input/output link in analysis chain
template<typename T, typename U>
class DataLink: public DataSink<T>, public SinkUser<U> {
public:
    /// pass through data flow signal
    void signal(datastream_signal_t s) override { SinkUser<U>::su_signal(s); }
};


/// Mix-in to add an input transform stage to a DataSink
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
        explicit _xfer(PreSink& _out): out(_out) { }
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
        PreTransform.setOwnsNext(false);
    }

    /// pass input to pre-filter
    void push(input_t& o) override { PreTransform.push(o); }
    /// pass through signals
    void signal(datastream_signal_t s) override { PreTransform.signal(s); }

protected:
    /// Override to handle pre-transformed input:
    virtual void _push(mid_t&) = 0;
    /// Override to handle signals through pre-transform
    virtual void _signal(datastream_signal_t) = 0;
};

#include "AnaIndex.hh"
template<typename T>
const _AnaIndex& SinkUser<T>::getSinkIdx() const {
    static AnaIndex<T> I;
    return I;
}

#endif
