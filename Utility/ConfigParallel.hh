/// \file ConfigParallel.hh Configurable parallelize-and-collate process
// Michaep P. Mendenhall, LLNL 2021

#ifndef CONFIGPARALLEL_HH
#define CONFIGPARALLEL_HH

#include "ConfigCollator.hh"
#include "ThreadBufferSink.hh"
#include "ClusteredWindow.hh"
#include <thread>

/// Type-independent re-casting base
class _ConfigParallel: public Configurable, public XMLProvider, public _SubSinkUser {
public:
    /// Constructor
    explicit _ConfigParallel(const Setting& S):
    Configurable(S), XMLProvider("Parallel") { }

protected:
    _ConfigCollator* myColl = nullptr;   ///< output collator
};

/// Configurable parallelize-and-collate process
template<typename T, class CLUST = Clusterer<T>>
class ConfigParallel: virtual public _ConfigParallel, public PreSink<CLUST> {
public:
    /// Constructor
    explicit ConfigParallel(const Setting& S): _ConfigParallel(S), PreSink<CLUST>(1000) {
        S.lookupValue("cluster_dt", this->PreTransform.cluster_dx);

        if(S.exists("next")) { // collated mode

            addParallel();
            tryAdd(vout.back()->getNext());

            subSinker = myColl = vends.back()->getSinkIdx().makeConfigCollator(S);

            if(myColl->nthreads <= 0) {
                vends.back()->_setNext(myColl->_getNext());
                subSinker = vout.back();
                return;
            }

            tryAdd(myColl);

            for(int i = 1; i < myColl->nthreads; ++i) addParallel();
            for(auto c: vends) myColl->connect_input(*c);

            myColl->launch_mythread();
            for(auto c: vout) c->launch_mythread();

        } else { // end in parallel chains without collation back to single thread

            int nth = std::thread::hardware_concurrency();
            S.lookupValue("nthreads", nth);
            optionalGlobalArg("nParallel", nth, "number of parallel chains (0 for unbuffered single-threaded mode)");
            do {
                vout.push_back(new ThreadBufferSink<T>(constructCfgObj<DataSink<T>>(Cfg["parallel"])));
            } while(--nth > 0);

            if(!nth) for(auto c: vout) c->launch_mythread();
        }

        if(vout.size()) tryAdd(vout.back()->getNext());
    }

    /// Destructor
    ~ConfigParallel() {
        for(auto o: vout) {
            if(o->checkRunning()) o->finish_mythread();
            delete o;
        }
        if(myColl && myColl->checkRunning()) myColl->finish_mythread();
        delete myColl;
    }

    /// add new parallel stream
    void addParallel() {
        vout.push_back(new ThreadBufferSink<T>(constructCfgObj<DataSink<T>>(Cfg["parallel"])));
        vends.push_back(_find_lastSink(vout.back()));
        vends.back()->ownsNext = false;
    }

    /// pass clustered inputs round-robin to parallel chains
    void _push(const typename CLUST::cluster_t& C) override {
        if(!vout.size()) return;
        auto o = vout[(outn++) % vout.size()];
        for(auto& p: C) o->push(p);
    }

    /// receive signal: pass to data chains, repeat to collator
    void signal(datastream_signal_t s) override {
        for(auto& o: vout) o->signal(s);
        if(myColl) myColl->signal(s);
    }

protected:
    size_t outn = 0;                    ///< round-robin output index
    vector<ThreadBufferSink<T>*> vout;  ///< outputs
    vector<_SinkUser*> vends;           ///< ends of parallel chains

    /// XML metadata output
    void _makeXML(XMLTag& X) override {
        if(!myColl) X.addAttr("nparallel", vout.size());
    }
};

#endif
