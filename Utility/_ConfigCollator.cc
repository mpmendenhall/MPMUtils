/// \file _ConfigCollator.cc

#include "ConfigCollator.hh"

void _ConfigCollator::run() {
    if(!Cfg.exists("prev")) throw std::runtime_error("Collator requires prev: input chain");
    if(nthreads <= 0) run_singlethread();
    else run_multithread();
}

void _ConfigCollator::run_singlethread() {
    C0 = constructCfgObj<Configurable>(Cfg["prev"]);
    tryAdd(C0);
    if(Cfg.exists("next")) _find_lastSink(C0)->createOutput(Cfg["next"]);
    C0->run();
}

void _ConfigCollator::run_multithread() {
    if(Cfg.exists("next")) createOutput(Cfg["next"]);

    vector<ConfigThreadWrapper*> chains;
    for(int i = 0; i < nthreads; ++i) {
        auto C = constructCfgObj<Configurable>(Cfg["prev"]);
        if(!chains.size()) { C0 = C; tryAdd(C0); }
        chains.push_back(new ConfigThreadWrapper(C, i));
        connect_input(*find_lastSink(C));
    }

    sigNext(DATASTREAM_START);
    this->launch_mythread();
    for(auto c: chains) c->launch_mythread();
    for(auto c: chains) c->finish_mythread();
    this->finish_mythread();

    for(auto c: chains) {
        if(c->C != C0) delete c->C;
        delete c;
    }
    sigNext(DATASTREAM_END);
}
