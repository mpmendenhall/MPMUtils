/// \file PrintClustered.hh Print organized by cluster
// -- Michael P. Mendenhall, LLNL 2021

#ifndef PRINTCLUSTERED_HH
#define PRINTCLUSTERED_HH

#include "XMLTag.hh"
#include "GlobalArgs.hh"
#include "ConfigFactory.hh"
#include "Clustered.hh"
#include "TermColor.hh"

/// Display organized by cluster
template<class C, typename T = const typename C::contents_t>
class PrintClustered: public SinkUser<T>, public PreSink<ClusterBuilder<C>>, public XMLProvider {
public:
    typedef ClusterBuilder<C> CB_t;
    typedef PreSink<CB_t> super_t;
    typedef typename super_t::presink_t::ordering_t ordering_t;
    using SinkUser<T>::nextSink;

    /// Constructor
    explicit PrintClustered(const Setting& S):
    super_t(20), XMLProvider("PrintClustered") {
        S.lookupValue("nskip", nskip);
        optionalGlobalArg("printskip", nskip, "cluster printout decimation factor");
        if(S.exists("next")) this->createOutput(S["next"]);
    }

    /// intercept and pass input objects
    void push(T& o) override {
        super_t::push(o);
        if(nextSink) nextSink->push(o);
    }

    /// show signals
    void signal(datastream_signal_t sig) override {
        printf(TERMFG_MAGENTA
        "-- datastream signal %i\n"
        TERMSGR_RESET, sig);
        this->PreTransform.signal(sig);
        SinkUser<T>::su_signal(sig);
    }

    int nskip = 1;          ///< skip fraction for less verbose output

protected:
    ordering_t t_prev_clust = {}; ///< previous cluster time

    /// ignore signals from clusterer
    void _signal(datastream_signal_t) override { }

    /// select clusters to view
    void _push(C& o) override {
        static int nc = 0;
        if(!(++nc % nskip)) {
            if(o.size()) {
                printf(TERMFG_BLUE "\n-- gap of %.3f us --\n\n" TERMSGR_RESET, (ordering_t(o[0]) - t_prev_clust)*1e-3);
                t_prev_clust = ordering_t(o.back());
            } else printf(TERMFG_RED "\n** empty cluster **\n" TERMSGR_RESET);

            dispClust(o);
        }
    }

    /// display cluster
    virtual void dispClust(const C& o) { o.display(); }
};

#endif
