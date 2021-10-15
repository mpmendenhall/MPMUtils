/// \file DataSinkTee.hh Configurable data sink splitter
// -- Michael P. Mendenhall, LLNL 2021

#ifndef DATASINKTEE_HH
#define DATASINKTEE_HH

#include "ConfigFactory.hh"
#include "XMLTag.hh"

/// Tee input to multiple configured sinks
template<typename T>
class DataSinkTee: public DataSink<T>, virtual public XMLProvider {
public:
    typedef T sink_t;
    typedef DataSink<T> dsink_t;

    /// Constructor, from config file
    explicit DataSinkTee(const Setting& S): XMLProvider("DataSinkTee") {
        if(!S.exists("next")) throw std::runtime_error("DataSinkTee missing 'next' outputs");
        auto& nxt = S["next"];
        if(nxt.isList()) for(auto& cfg: nxt) sinks.push_back(constructCfgObj<dsink_t>(cfg, ""));
        else sinks.push_back(constructCfgObj<dsink_t>(nxt, ""));
        for(auto s: sinks) tryAdd(s);
    }

    /// Destructor
    ~DataSinkTee() { for(auto s: sinks) delete s; }

    /// take instance of object
    void push(sink_t& x) override { for(auto s: sinks) s->push(x); }
    /// accept data flow signal
    void signal(datastream_signal_t sig) override { for(auto s: sinks) s->signal(sig); }

protected:
    vector<dsink_t*> sinks; ///< output sinks
};

#endif
