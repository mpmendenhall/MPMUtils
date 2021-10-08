/// \file ConfigCollator.hh Configturation-buildable Collator object
// Michael P. Mendenhall, LLNL 2021

#ifndef CONFIGCOLLATOR_HH
#define CONFIGCOLLATOR_HH

#include "_Collator.hh"
#include "ConfigThreader.hh"
#include "GlobalArgs.hh"
#include "XMLTag.hh"
#include <thread>

/// Type-independent re-casting base
class _ConfigCollator: public Configurable, public XMLProvider, virtual public _Collator {
public:
    /// Constructor
    explicit _ConfigCollator(const Setting& S):
    Configurable(S), XMLProvider("Collator"), nthreads(std::thread::hardware_concurrency()) {
        S.lookupValue("ninputs", nthreads);
        optionalGlobalArg("nCollimate", nthreads, "number of parallel collated processes (0 for single-threaded)");
    }

    /// Destructor
    ~_ConfigCollator() { delete C0; }

    int nthreads;               ///< number of separate input threads (0 for single-threaded)
    Configurable* C0 = nullptr; ///< representative input chain head

    /// XML output info
    void _makeXML(XMLTag& X) override {
        X.addAttr("nthreads", nthreads);
    }
};

#include "Collator.hh"

/// Configturation-buildable Collator object
template<class T>
class ConfigCollator: public _ConfigCollator, public Collator<T> {
public:
    using Collator<T>::nextSink;

    /// Constructor
    explicit ConfigCollator(const Setting& S): _ConfigCollator(S) {
        if(S.exists("next")) {
            nextSink = constructCfgObj<DataSink<T>>(S["next"]);
            tryAdd(nextSink);
        }
    }

    /// Run as top-level object
    void run() override {
        if(!nextSink) throw std::runtime_error("Collator requires next: output");
        if(!Cfg.exists("prev")) throw std::runtime_error("Collator requires prev: input chain");

        // single-threaded mode
        if(nthreads <= 0) { run_singlethread(); return; }

        vector<ConfigThreader*> chains;
        for(int i = 0; i < nthreads; ++i) {
            auto C = constructCfgObj<Configurable>(Cfg["prev"]);
            if(!chains.size()) { C0 = C; tryAdd(C0); }
            chains.push_back(new ConfigThreader(*C));
            connect_input(*find_lastSinkUser<T>(C));
        }

        nextSink->signal(DATASTREAM_START);
        this->launch_mythread();
        for(auto c: chains) c->launch_mythread();
        for(auto c: chains) c->finish_mythread();
        this->finish_mythread();

        for(auto c: chains) {
            if(&c->C != C0) delete &c->C;
            delete c;
        }
        nextSink->signal(DATASTREAM_END);
    }

    /// single-threaded run mode
    void run_singlethread() {
        C0 = constructCfgObj<Configurable>(Cfg["prev"]);
        tryAdd(C0);
        auto inSrc = find_lastSinkUser<T>(C0);
        inSrc->getNext() = nextSink;
        inSrc->ownsNext = false;
        C0->run();
    }
};

/// Registration in AnaIndex
template<typename T>
_ConfigCollator* AnaIndex<T>::makeConfigCollator(const Setting& S) const { return new ConfigCollator<T>(S); }

#endif
