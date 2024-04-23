/// @file SignalHooks.hh Configurable action on datastream signals
// -- Michael P. Mendenhall, LLNL 2024

#ifndef SIGNALHOOKS_HH
#define SIGNALHOOKS_HH

#include "SignalSink.hh"
#include "XMLTag.hh"
#include "ConfigFactory.hh"

/// Base configurable signal receiver
class ConfigSignals: public Configurable, public SignalSink, public XMLProvider {
public:
    /// Constructor
    explicit ConfigSignals(const Setting& S);
    /// Destructor
    ~ConfigSignals() { delete nextSig; }

    /// accept data flow signal
    void signal(datastream_signal_t s) override { if(nextSig) nextSig->signal(s); }

protected:
    SignalSink* nextSig = nullptr;
};

#endif
