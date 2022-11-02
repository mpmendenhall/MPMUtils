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
template<class CB, typename T = const typename CB::cluster_t::contents_t>
class PrintClustered: public SinkUser<T>, public PreSink<CB>, public XMLProvider {
public:
    typedef CB CB_t;
    typedef PreSink<CB_t> super_t;
    typedef typename super_t::presink_t::ordering_t ordering_t;
    using SinkUser<T>::nextSink;
    typedef typename CB_t::cluster_t cluster_t;

    /// Constructor
    explicit PrintClustered(const Setting& S):
    super_t(20), XMLProvider("PrintClustered") {
        S.lookupValue("nskip", nskip);
        optionalGlobalArg("printskip", nskip, "cluster printout decimation factor");
        S.lookupValue("npause", npause);
        optionalGlobalArg("npause", npause, "pause display after every n clusters shown");
        if(S.exists("next")) this->createOutput(S["next"]);
        S.lookupValue("tcluster", this->PreTransform.cluster_dx);
    }

    /// intercept and pass input objects
    void push(T& o) override {
        super_t::push(o);
        if(nextSink) nextSink->push(o);
    }

    /// show signals
    void signal(datastream_signal_t sig) override {
        printf(TERMFG_MAGENTA "-- datastream signal '%s'\n"
               TERMSGR_RESET, signal_name(sig).c_str());
        this->PreTransform.signal(sig);
        SinkUser<T>::su_signal(sig);
    }

    int nskip = 1;  ///< skip fraction for less verbose output
    int npause = 0; ///< pause after showing this many (if nonzero)

protected:
    ordering_t t_prev_clust = {}; ///< previous cluster time

    /// ignore signals from clusterer
    void _signal(datastream_signal_t) override { }

    /// select clusters to view
    void _push(cluster_t& o) override {
        static int nc = 0;
        if(++nc % nskip) return;

        if(o.size()) {
            printf(TERMFG_BLUE "\n-- gap of %.3f us --" TERMSGR_RESET "\n", (ordering_t(o[0]) - t_prev_clust)*1e-3);
            t_prev_clust = ordering_t(o.back());
        } else printf(TERMFG_RED "\n** empty cluster **" TERMSGR_RESET "\n");

        dispClust(o);

        if(npause > 0) {
            static int nshow = 0;
            if(!(++nshow % npause)) {
                printf(TERMFG_YELLOW "\n------------------- Press [enter] to continue... -------------------------" TERMSGR_RESET "\n");
                getchar();
            }
        }
    }

    /// display cluster
    virtual void dispClust(const cluster_t& o) { dispObj(o); }
};

#endif
