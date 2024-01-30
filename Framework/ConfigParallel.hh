/// @file ConfigParallel.hh Configurable parallelize-and-collate process
// Michaep P. Mendenhall, LLNL 2021

#ifndef CONFIGPARALLEL_HH
#define CONFIGPARALLEL_HH

#include "ConfigCollator.hh"
#include "ThreadBufferSink.hh"
#include "ClusteredWindow.hh"
#include "AnaIndex.hh"
#include <thread>

/// Type-independent re-casting base
class _ConfigParallel: public Configurable, public ThreadManager, public XMLProvider, public _SubSinkUser {
public:
    /// Constructor
    explicit _ConfigParallel(const Setting& S);

    /// Destructor
    ~_ConfigParallel() { delete keep_me; delete myColl; }

    /// if run at top scope...
    void run() override;

protected:
    int nparallel = 0;                  ///< number of parallel threads to run
    vector<_SinkUser*> vends;           ///< ends of parallel chains
    _ConfigCollator* myColl = nullptr;  ///< output collator
    Threadworker* keep_me = nullptr;    ///< keep one example chain for XML output

    /// Callback on thread completion
    void on_thread_completed(Threadworker* t) override { if(t != keep_me) delete t; }

    /// construct and attach appropriate myColl to vends
    void makeCollator();

    /// XML metadata output
    void _makeXML(XMLTag& X) override { X.addAttr("nparallel", nparallel); }
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
            for(int i = 0; i < nparallel; ++i) addParallel();
            tryAdd(vout.back()->getNext());
            makeCollator();
            if(nparallel > 0) subSinker = myColl;
            else subSinker = vout.back();

            myColl->launch_mythread();
            for(auto c: vout) c->launch_mythread();

        } else { // end in parallel chains without collation back to single thread

            int nth = nparallel;
            do {
                vout.push_back(new ThreadBufferSink<T>(Cfg["parallel"]));
                vout.back()->worker_id = --nth;
            } while(nth > 0);

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
    }

    /// add new parallel stream
    void addParallel() {
        vout.push_back(new ThreadBufferSink<T>(Cfg["parallel"]));
        vout.back()->worker_id = vout.size()-1;
        vends.push_back(_find_lastSink(vout.back()));
        vends.back()->setOwnsNext(false);
    }

    /// pass clustered inputs round-robin to parallel chains
    void _push(typename CLUST::cluster_t& C) override {
        if(!vout.size()) return;
        auto o = vout[(outn++) % vout.size()];
        for(auto& p: C) o->push(p);
    }

    /// handle signals through pre-transform
    void _signal(datastream_signal_t s) override {
        for(auto& o: vout) o->signal(s);
        if(myColl) myColl->signal(s);
    }

protected:
    size_t outn = 0;                    ///< round-robin output index
    vector<ThreadBufferSink<T>*> vout;  ///< outputs to parallel chains
};

#endif
