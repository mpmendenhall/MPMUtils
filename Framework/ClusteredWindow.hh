/// \file ClusteredWindow.hh Short-range clustering organization
// -- Michael P. Mendenhall, LLNL 2020

#ifndef CLUSTEREDWINDOW_HH
#define CLUSTEREDWINDOW_HH

#include "OrderedWindow.hh"
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
    /// max value for ordering operator
    static constexpr ordering_t order_max = std::numeric_limits<ordering_t>::max();


    /// Constructor
    explicit Cluster(ordering_t w = {}): dx(w) { }
    /// forbid direct assignment
    const Cluster& operator=(const Cluster& C) = delete;
    /// Polymorphic destructor
    virtual ~Cluster() { }

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
        for(auto& o: *this) {
            printf("\t");
            dispObj(o);
        }
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
class ClusterBuilder: public DataLink<const typename C::T, const C> {
public:
    typedef C cluster_t;
    typedef typename cluster_t::T T;
    typedef typename cluster_t::ordering_t ordering_t;

    /// Constructor
    explicit ClusterBuilder(ordering_t cdx): cluster_dx(cdx) {  }

    /// accept data flow signal
    void signal(datastream_signal_t sig) override {
        if(sig >= DATASTREAM_FLUSH) {
            completeCluster();
            t_prev = -C::order_max;
        }
        if(this->nextSink) this->nextSink->signal(sig);
    }

    /// push currentC into window
    void completeCluster() {
        currentC.close();
        if(checkCluster(currentC) && this->nextSink) this->nextSink->push(currentC);
        currentC.clear();
    }

    /// add object to newest cluster (or start newer cluster), assuming responsibility for deletion
    void push(const T& o) override {
        ordering_t t(o);
        if(!(t_prev <= t)) {
            dispObj(currentC);
            printf("t_prev = %g\n", t_prev);
            dispObj(o);
            throw std::runtime_error("Out-of-order item received for clustering");
        }
        t_prev = t;
        currentC.dx = cluster_dx;
        if(!currentC.tryAdd(o)) {
            completeCluster();
            if(!currentC.tryAdd(o)) throw -1;
        }
    }

    ordering_t cluster_dx{};            ///< time spread for cluster identification

protected:
    /// examine and decide whether to include cluster
    virtual bool checkCluster(cluster_t& o) { return o.size(); }

    cluster_t currentC{};               ///< cluster currently being built
    ordering_t t_prev = -C::order_max;  ///< previous item arrival
};

/// Wrap ClusterBuilder in OrderedWindow
template<class CB>
class CBWindow: public CB, public OrderedWindow<typename CB::cluster_t, typename CB::ordering_t> {
public:
    typedef typename CB::cluster_t cluster_t;
    typedef typename cluster_t::T T;
    typedef typename cluster_t::ordering_t ordering_t;
    typedef T sink_t;

    /// Constuctor with pass-through args
    template<typename... Args>
    explicit CBWindow(ordering_t dw, Args&&... a):
    CB(std::forward<Args>(a)...), OrderedWindow<cluster_t, ordering_t>(dw) { CB::nextSink = this; }

    using CB::signal;
    /// special override for non-loopy signalling
    void signal(datastream_signal_t sig) override {
        if(sig >= DATASTREAM_FLUSH) CB::completeCluster();
        OrderedWindow<cluster_t, ordering_t>::signal(sig);
    }

    using CB::push; // push T
    using OrderedWindow<cluster_t, ordering_t>::push; // push cluster_t

protected:
    using OrderedWindow<cluster_t, ordering_t>::signal; // hide from direct calls
};

/// OrderedWindow of "clustered" objects
template<class C>
using ClusteredWindow = CBWindow<ClusterBuilder<C>>;

#endif
