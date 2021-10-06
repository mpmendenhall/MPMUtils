/// \file ConfigCollator.hh Configturation-buildable Collator object
// Michael P. Mendenhall, LLNL 2021

#ifndef CONFIGCOLLATOR_HH
#define CONFIGCOLLATOR_HH

#include "Collator.hh"
#include "ConfigFactory.hh"
#include "GlobalArgs.hh"
#include "XMLTag.hh"
#include "fftwx.hh"

/// wrapper to run Configurable in its own thread
class ConfigThreader: public Threadworker {
public:
    /// Constructor
    ConfigThreader(Configurable& _C): C(_C) { }
    /// run in thread
    void threadjob() override { C.run(); }
    /// the Configurable to run
    Configurable& C;
};

/// Configturation-buildable Collator object
template<class T>
class ConfigCollator: public Collator<T>, public Configurable, public XMLProvider  {
public:
    using Collator<T>::nextSink;
    typedef typename Collator<T>::MOqInput qadaptor_t;
    int nthreads = 0;           ///< number of separate input threads (0 for single-threaded)
    Configurable* C0 = nullptr; ///< representative input chain head

    /// Constructor
    explicit ConfigCollator(const Setting& S): Configurable(S), XMLProvider("Collator") {
        if(S.exists("next")) {
            nextSink = constructCfgObj<DataSink<T>>(S["next"]);
            tryAdd(nextSink);
        }
        S.lookupValue("ninputs", nthreads);
        optionalGlobalArg("nCollimate", nthreads, "number of parallel collated processes (0 for single-threaded)");

    }

    /// Destructor
    ~ConfigCollator() { delete C0; }

    /// Run as top-level object
    void run() override {
        if(!nextSink) throw std::runtime_error("Collator requires next: output");
        if(!Cfg.exists("prev")) throw std::runtime_error("Collator requires prev: input chain");

        // single-threaded mode
        if(nthreads <= 0) { run_singlethread(); return; }

        vector<ConfigThreader*> chains;
        vector<qadaptor_t> adapters;
        for(int i = 0; i < nthreads; ++i) adapters.emplace_back(*this);
        for(auto& a: adapters) {
            auto C = constructCfgObj<Configurable>(Cfg["prev"]);
            if(!chains.size()) { C0 = C; tryAdd(C0); }
            chains.push_back(new ConfigThreader(*C));
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
            if(&c->C != C0) delete &c->C;
            delete c;
        }
        nextSink->signal(DATASTREAM_END);
    }

    /// single-threaded run mode
    void run_singlethread() {
        C0 = constructCfgObj<Configurable>(Cfg["prev"]);
        tryAdd(C0);
        auto inSrc = find_lastSink<T>(C0);
        inSrc->nextSink = nextSink;
        inSrc->ownsNext = false;
        C0->run();
    }

    /// XML output info
    void _makeXML(XMLTag& X) override {
        X.addAttr("nthreads", nthreads);

    }
};

#endif
