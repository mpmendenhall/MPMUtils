/// \file ClusteredWindow.hh Short-range clustering organization
// Michael P. Mendenhall, LLNL 2019

#ifndef CLUSTEREDWINDOW_HH
#define CLUSTEREDWINDOW_HH

#include "OrderedWindow.hh"
#include <cmath> // for fabs
#include <vector>
using std::vector;

/// "Cluster" base class
template<class _T, typename _ordering_t = double>
class Cluster: public vector<_T> {
public:
    typedef _T T;
    typedef _ordering_t ordering_t;
    using vector<T>::size;
    using vector<T>::back;
    using vector<T>::begin;
    using vector<T>::end;

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
    virtual void clear() { vector<T>::clear(); }

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
    virtual void append(const T& o) { this->push_back(o); }

    ordering_t x_median = 0;    ///< representative median object position
};

/// OrderedWindow of "clustered" objects
template<class C>
class ClusteredWindow: public DataSink<typename C::T>, public OrderedWindow<C, typename C::ordering_t> {
public:
    typedef typename C::T T;
    typedef typename C::ordering_t ordering_t;

    /// Constructor
    ClusteredWindow(ordering_t cdx, ordering_t dw): OrderedWindow<C>(dw), cluster_dx(cdx) { currentC.dx = cluster_dx; }

    /// clear remaining objects through window
    void flush() override {
        completeCluster();
        OrderedWindow<C>::flush();
    }

    /// add object to newest cluster (or start newer cluster), assuming responsibility for deletion
    void push(const T& oo) override {
        auto o = oo;
        processSingle(o);
        if(!currentC.tryAdd(o)) {
            completeCluster();
            if(!currentC.tryAdd(o)) throw;
        }
    }

    ordering_t cluster_dx;  ///< time spread for cluster identification

protected:

    /// process new single element before adding to clustering sequence --- Subclass me!
    virtual void processSingle(T&) { }
    /// decide whether to include cluster
    virtual bool includeCluster(const C&) const { return true; }

    C currentC; ///< cluster currently being built

    /// push currentC into window
    virtual void completeCluster() {
        currentC.close();
        if(currentC.size() && includeCluster(currentC)) this->OrderedWindow<C>::push(currentC);
        currentC.clear();
    }
};

#endif
