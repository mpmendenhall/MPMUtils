/// @file

#include "SignalHooks.hh"

ConfigSignals::ConfigSignals(const Setting& S):
Configurable(S), XMLProvider("ConfigSignals") {
    if(Cfg.show_exists("nextSig", "next recipient for dataflow signals")) {
        nextSig = constructCfgObj<SignalSink>(Cfg["nextSig"], "ConfigSignals");
        tryAdd(nextSig);
    }
}

REGISTER_CONFIG(ConfigSignals, SignalSink)

