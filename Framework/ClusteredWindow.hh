/// \file ClusteredWindow.hh Short-range clustering organization
// Michael P. Mendenhall, LLNL 2019

#ifndef CLUSTEREDWINDOW_HH
#define CLUSTEREDWINDOW_HH

#include "OrderedWindow.hh"
#include <cmath> // for fabs
#include <vector>
using std::vector;

/// "Cluster" base class
template<class _T, typename _ordering_t = typename _T::ordering_t>
class Cluster: public vector<_T> {
public:
    typedef _T T;
    typedef _ordering_t ordering_t;
    typedef vector<T> super;
    using super::size;
    using super::back;
    using super::begin;
    using super::end;
    using super::push_back;

    /// Constructor
    Cluster(ordering_t w = 0): dx(w) { }
    /// Destructor
    virtual ~Cluster() { clear(); }

    /// Get ordering parameter
    explicit operator ordering_t() const { return x_median; }

    /// check cluster equality: simple check on median!
    bool operator==(const Cluster& C) const { return x_median == C.x_median; }
    /// check inequality
    bool operator!=(const Cluster& C) const { return !(*this == C); }

    /// Get cluster time spread from first to last
    ordering_t getWidth() const { return size()? ordering_t(back()) - ordering_t((*this)[0]) : 0; }

    /// print cluster information
    virtual void display(ordering_t x0 = 0) const {
        printf("Cluster with %zu objects at t = %g (max spacing %g)\n", size(), x_median - x0, dx);
    }

    /// Perform analysis at completion of cluster. Override to add more fancy calculations.
    virtual void close() { x_median = size()? ordering_t((*this)[size()/2]) : 0; }

    /// Clear contents
    virtual void clear() { super::clear(); }

    /// sort contents by ordering parameter
    void sort() { std::sort(begin(), end(), [this](const T& a, const T& b) { return ordering_t(a) < ordering_t(b); }); }

    ordering_t dx;  ///< clustering interval; modify in derived classes.

    // check whether object time would fall in cluster
    bool inClusterRange(const T& O) { return !size() || fabs(ordering_t(O) - ordering_t(back())) <= dx; }

    /// check whether supplied object falls within clustering time of previous event; add to cluster if so.
    bool tryAdd(const T& O) {
        if(!inClusterRange(O)) return false;
        append(O);
        return true;
    }

protected:
    /// append item to cluster. Override to add more fancy calculations.
    virtual void append(const T& o) { push_back(o); }

    ordering_t x_median = 0;    ///< representative median object position
};


/// Cluster builder
template<class C>
class ClusterBuilder: public DataSink<typename C::T> {
public:
    typedef C cluster_t;
    typedef typename cluster_t::T T;
    typedef typename cluster_t::ordering_t ordering_t;

    /// Constructor
    ClusterBuilder(ordering_t cdx): cluster_dx(cdx) {  }

    /// accept data flow signal
    void signal(datastream_signal_t sig) override {
        if(sig >= DATASTREAM_FLUSH) completeCluster();
        if(clustOut) clustOut->signal(sig);
    }

    /// push currentC into window
    void completeCluster() {
        currentC.close();
        if(currentC.size() && checkCluster(currentC) && clustOut) clustOut->push(currentC);
        currentC.clear();
    }

    /// add object to newest cluster (or start newer cluster), assuming responsibility for deletion
    void push(const T& oo) override {
        currentC.dx = cluster_dx;
        auto o = oo;
        processSingle(o);
        if(!currentC.tryAdd(o)) {
            completeCluster();
            if(!currentC.tryAdd(o)) throw;
        }
    }

    ordering_t cluster_dx;                      ///< time spread for cluster identification
    DataSink<cluster_t>* clustOut = nullptr;    ///< clustered objects recipient

protected:
    /// process new single element before adding to clustering sequence --- Subclass me!
    virtual void processSingle(T&) { }
    /// examine and decide whether to include cluster
    virtual bool checkCluster(cluster_t&) { return true; }

    cluster_t currentC; ///< cluster currently being built
};

/// Wrap ClusterBuilder in OrderedWindow
template<class CB>
class CBWindow: public CB, public OrderedWindow<typename CB::cluster_t, typename CB::ordering_t> {
public:
    typedef typename CB::cluster_t cluster_t;
    typedef typename cluster_t::T T;
    typedef typename cluster_t::ordering_t ordering_t;

    /// Basic constructor
    CBWindow(ordering_t cdx, ordering_t dw):
    CB(cdx), OrderedWindow<cluster_t, ordering_t>(dw) {
        CB::clustOut = this;
    }

    /// Special-purpose constructor (specialize me!)
    template<typename... Args>
    CBWindow(Args&&...) { }

    /// Non-loopy signal passthrough
    using CB::signal;
    void signal(datastream_signal_t sig) override {
        if(sig >= DATASTREAM_FLUSH) this->completeCluster();
        OrderedWindow<cluster_t, ordering_t>::signal(sig);
    }

    using CB::push;
    using OrderedWindow<cluster_t, ordering_t>::push;
};

/// OrderedWindow of "clustered" objects
template<class C>
using ClusteredWindow = CBWindow<ClusterBuilder<C>>;

#endif
