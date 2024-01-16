/// @file _ConfigParallel.cc

#include "ConfigParallel.hh"

_ConfigParallel::_ConfigParallel(const ConfigInfo_t& S):
Configurable(S), XMLProvider("Parallel"), nparallel(std::thread::hardware_concurrency()) {
    Cfg.lookupValue("nthreads", nparallel);
    optionalGlobalArg("nParallel", nparallel, "number of parallel chains");
}

void _ConfigParallel::makeCollator() {
    if(!vends.size()) return;
    myColl = vends.back()->getSinkIdx().makeConfigCollator(*Cfg);
    tryAdd(myColl);

    if(nparallel <= 0) {
        if(vends.size() != 1) throw std::logic_error("multiple parallels in single-threaded mode");
        vends.back()->_setNext(myColl->_getNext());
        vends.back()->setOwnsNext(false);
    } else for(auto c: vends) myColl->connect_input(*c);
}

void _ConfigParallel::run() {
    if(!mythreads.size()) {
        if(!Cfg.exists("parallel")) return;

        int nth = nparallel;
        do {
            auto CT = new ConfigThreadWrapper(constructCfgObj<Configurable>(Cfg["parallel"], ""), --nth);
            add_thread(CT);
            if(Cfg.exists("next")) vends.push_back(_find_lastSink(CT->C));
            if(!keep_me) {
                tryAdd(CT->C);
                keep_me = CT;
            }
        } while(nth > 0);

        if(Cfg.exists("next")) { // produces output (to be collated in multithreaded mode)
            if(nparallel > 0) makeCollator();
            else vends.back()->createOutput(Cfg["next"]);
        }
    }

    if(nparallel <= 0) {
        (*mythreads.begin())->run_here();
        purge_pending();
    } else {
        if(myColl) myColl->launch_mythread();
        run_here();
        if(myColl) myColl->finish_mythread();
    }
}

typedef _ConfigParallel Parallel;
REGISTER_CONFIGURABLE(Parallel)
