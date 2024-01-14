/// @file PrintSink.hh Display datastream contents
// -- Michael P. Mendenhall, LLNL 2021

#ifndef PRINTSINK_HH
#define PRINTSINK_HH

#include "SinkUser.hh"
#include "TermColor.hh"
#include "XMLTag.hh"
#include "GlobalArgs.hh"

/// Display each received object
template<class T>
class PrintSink: public DataLink<T,T>, public XMLProvider  {
public:
    using DataLink<T,T>::nextSink;

    /// Constructor
    explicit PrintSink(const Setting& S): XMLProvider("PrintSink") {
        S.lookupValue("nskip", nskip);
        optionalGlobalArg("printskip", nskip, "printout decimation factor");
        if(S.exists("next")) this->createOutput(S["next"]);
    }
    /// take instance of object
    void push(T& o) override {
        static int n = 0;
        if(!(++n % nskip)) o.display();
        if(nextSink) nextSink->push(o);
    }

    /// show signals
    void signal(datastream_signal_t sig) override {
        printf(TERMFG_MAGENTA "-- datastream signal '%s'\n"
               TERMSGR_RESET, signal_name(sig).c_str());
        DataLink<T,T>::signal(sig);
    }

    int nskip = 1;
};



#endif
