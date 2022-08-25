/// \file Clustered.hh Short-range clustering organization
// -- Michael P. Mendenhall, LLNL 2021

#ifndef CLUSTERED_HH
#define CLUSTERED_HH

#include "SinkUser.hh"
#include <cmath> // for fabs()

/// "Cluster" base class
template<class _T, typename _ordering_t = typename _T::ordering_t>
class Cluster: public vector<typename std::remove_const<_T>::type> {
public:
    typedef typename std::remove_const<_T>::type contents_t;
    typedef _ordering_t ordering_t;
    typedef vector<contents_t> super_t;
    using super_t::size;
    using super_t::back;
    using super_t::begin;
    using super_t::end;
    using super_t::push_back;

    /// max value for ordering operator
    static constexpr ordering_t order_max = std::numeric_limits<ordering_t>::max();

    /// Constructor
    explicit Cluster(ordering_t w = {}): dx(w) { }
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
    virtual void clear() { super_t::clear(); }

    /// sort contents by ordering parameter
    void sort() { std::sort(begin(), end(), [this](const contents_t& a, const contents_t& b) { return ordering_t(a) < ordering_t(b); }); }

    ordering_t dx;  ///< clustering interval; modify in derived classes.

    // check whether object time would fall in cluster
    bool inClusterRange(const contents_t& O) { return !size() || fabs(ordering_t(O) - ordering_t(back())) <= dx; }

    /// check whether supplied object falls within clustering time of previous event; add to cluster if so.
    bool tryAdd(const contents_t& O) {
        if(!inClusterRange(O)) return false;
        append(O);
        return true;
    }

protected:
    /// append item to cluster. Override to add more fancy calculations.
    virtual void append(const contents_t& o) { push_back(o); }

    ordering_t x_median = 0;    ///< representative median object position
};


/// Cluster builder; input always const, output const-ness determined from C
template<class C>
class ClusterBuilder: public DataLink<const typename C::contents_t, C> {
public:
    typedef C cluster_t;
    typedef typename std::remove_const<cluster_t>::type cmut_t;
    using typename DataLink<const typename C::contents_t, C>::sink_t;
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
    void push(sink_t& o) override {
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
            if(!currentC.tryAdd(o)) throw std::logic_error("Empty cluster refused first item");
        }
    }

    ordering_t cluster_dx{};            ///< time spread for cluster identification

protected:
    /// inspect before passing along
    virtual bool checkCluster(cluster_t&) { return true; }

    cmut_t currentC{};                  ///< cluster currently being built
    ordering_t t_prev = -C::order_max;  ///< previous item arrival
};

/// Default clustering type
template<class T>
using Clusterer = ClusterBuilder<Cluster<T>>;

#endif
