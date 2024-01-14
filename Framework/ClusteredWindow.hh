/// @file ClusteredWindow.hh Short-range clustering organization
// -- Michael P. Mendenhall, LLNL 2020

#ifndef CLUSTEREDWINDOW_HH
#define CLUSTEREDWINDOW_HH

#include "OrderedWindow.hh"
#include "Clustered.hh"

/// Wrap ClusterBuilder in OrderedWindow
template<class CB>
class CBWindow: public PreSink<CB>,
public OrderedWindow<typename CB::cluster_t> {
public:
    typedef CB clustbuilder_t;
    typedef typename clustbuilder_t::cluster_t cluster_t;
    typedef typename cluster_t::ordering_t ordering_t;
    typedef OrderedWindow<cluster_t> window_t;

    /// Constuctor with pass-through args
    template<typename... Args>
    explicit CBWindow(ordering_t dw, Args&&... a):
    PreSink<CB>(std::forward<Args>(a)...), window_t(dw) { }

    using PreSink<CB>::push;

    /// receive signals
    void signal(datastream_signal_t s) override { PreSink<CB>::signal(s); }

protected:
    using OrderedWindow<cluster_t>::push;
    //using OrderedWindow<cluster_t>::signal;

    /// examine and decide whether to include cluster
    virtual bool checkCluster(cluster_t& o) { return o.size(); }

    /// Receive pre-transformed input:
    void _push(cluster_t& C) override { window_t::push(C); }
    /// receive back signals
    void _signal(datastream_signal_t s) override { window_t::signal(s); }
};

/// OrderedWindow of "clustered" objects
template<class C>
using ClusteredWindow = CBWindow<ClusterBuilder<C>>;

#endif
