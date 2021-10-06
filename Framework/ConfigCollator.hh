/// \file ConfigCollator.hh Configturation-buildable Collator object
// Michael P. Mendenhall, LLNL 2021

#ifndef CONFIGCOLLATOR_HH
#define CONFIGCOLLATOR_HH

#include "Collator.hh"
#include "ConfigFactory.hh"
#include "GlobalArgs.hh"
#include "XMLTag.hh"

/// wrapper to run Configurable in its own thread
class ConfigThreader: public Threadworker {
public:
    /// Constructor
    ConfigThreader(Configurable* _C): C(_C) { }
    /// run in thread
    void threadjob() override { if(C) C->run(); }
    /// the Configurable to run
    Configurable* C = nullptr;
};

/// Configturation-buildable Collator object
template<class T>
class ConfigCollator: public Collator<T>, public Configurable, public XMLProvider  {
public:
    using Collator<T>::nextSink;
    typedef typename Collator<T>::MOqInput qadaptor_t;

    /// Constructor
    explicit ConfigCollator(const Setting& S): Configurable(S), XMLProvider("Collator") {
        if(S.exists("next")) {
            nextSink = constructCfgObj<DataSink<T>>(S["next"]);
            tryAdd(nextSink);
        }
    }

    /// Destructor
    ~ConfigCollator() { delete C0; }

    /// Run as top-level object
    void run() override {
        if(!nextSink) throw std::runtime_error("Collator requires next: output");
        if(!Cfg.exists("prev")) throw std::runtime_error("Collator requires prev: input chain");

        // number of parallel input chains
        int nIn = 0;
        Cfg.lookupValue("ninputs", nIn);
        optionalGlobalArg("nCollimate", nIn, "number of parallel collated processes (0 for single-threaded)");

        // single-threaded mode
        if(nIn <= 0) { run_singlethread(); return; }

        vector<ConfigThreader*> chains;
        vector<qadaptor_t> adapters(nIn, *this);
        for(auto& a: adapters) {
            auto C = constructCfgObj<Configurable>(Cfg["prev"]);
            if(!chains.size()) { C0 = C; tryAdd(C0); }
            chains.push_back(new ConfigThreader(C));
            a.inSrc = find_lastSink<T>(C);
            a.inSrc->nextSink = &a;
            a.inSrc->ownsNext = false;
        }

        nextSink->signal(DATASTREAM_START);
        this->launch_mythread();
        for(auto c: chains) c->launch_mythread();
        for(auto c: chains) c->finish_mythread();
        this->finish_mythread();

        for(auto c: chains) {
            if(c->C != C0) delete c->C;
            delete c;
        }
        nextSink->signal(DATASTREAM_END);
    }

    void run_singlethread() {
        C0 = constructCfgObj<Configurable>(Cfg["prev"]);
        tryAdd(C0);
        auto inSrc = find_lastSink<T>(C0);
        inSrc->nextSink = nextSink;
        inSrc->ownsNext = false;
        C0->run();
    }

    Configurable* C0 = nullptr; ///< representative input
};

#endif
