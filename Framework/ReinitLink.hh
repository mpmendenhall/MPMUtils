/// @file ReinitLink.hh Re-initialize downstream chain on datastream start/stop
// -- Michael P. Mendenhall, LLNL 2024

#ifndef REINITLINK_HH
#define REINITLINK_HH

#include "SinkUser.hh"
#include "XMLTag.hh"

/// Re-initialize downstream chain on datastream start/stop
template<typename T>
class ReinitLink: public DataLink<T,T>, public XMLProvider, public Configurable {
public:
    typedef DataLink<T,T> dlink_t;
    using dlink_t::nextSink;
    typedef T sink_t;

    /// Constructor, from config file
    explicit ReinitLink(const Setting& S):
    XMLProvider("ReinitLink"), Configurable(S) { }

    /// take instance of object
    void push(sink_t& x) override { if(nextSink) nextSink->push(x); }

    /// accept data flow signal
    void signal(datastream_signal_t sig) override {
        if(sig == DATASTREAM_INIT || sig == DATASTREAM_REINIT) {
            if(nextSink) {
                nextSink->signal(DATASTREAM_END);
                tryRemove(nextSink);
                delete nextSink;
                nextSink = nullptr;
                printf("\nReinitializing downstream analysis\n\n");
            } else printf("\nInitializing downstream analysis\n\n");
            if(Cfg.show_exists("next", "ReinitLink downstream analysis chain")) this->createOutput(Cfg["next"]);
            dlink_t::signal(DATASTREAM_INIT);
        } else dlink_t::signal(sig);

        if(sig == DATASTREAM_END) {
            printf("\nClosing downstream analysis\n\n");
            tryRemove(nextSink);
            delete nextSink;
            nextSink = nullptr;
        }
    }
};

#endif
