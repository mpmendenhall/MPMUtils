/// \file ClusteredWindow.hh Short-range clustering organization
// Michael P. Mendenhall, LLNL 2018

#ifndef CLUSTEREDWINDOW_HH
#define CLUSTEREDWINDOW_HH

#include "OrderedWindow.hh"
#include "AllocPool.hh"
#include <cmath> // for fabs
#include <algorithm> // for std::sort

/// "Cluster" base class; responsible for contents memory management.
template<class TT>
class Cluster: public vector<TT*>, public AllocPool<TT> {
public:
    /// convenience typedef for reference outside class
    typedef TT T;

    /// Constructor
    Cluster(double w = 0): dx(w) { }
    /// Destructor; please clear before!
    virtual ~Cluster() { this->clear(); }

    /// Get ordering parameter
    operator double() const { return x_median; }

    /// Get cluster time spread from first to last
    double getWidth() const { return this->size()? order(*(this->back())) - order(*((*this)[0])) : 0; }

    /// print cluster information
    virtual void display(double x0 = 0) const {
        printf("Cluster with %zu objects at t = %g (max spacing %g)\n", this->size(), x_median - x0, dx);
    }

    /// Perform analysis at completion of cluster. Override to add more fancy calculations.
    virtual void close() { x_median = this->size()? order(*(*this)[this->size()/2]) : 0; }

    /// sort contents by ordering parameter
    void sort() { std::sort(this->begin(), this->end(), [this](T* a, T* b) { return this->order(*a) < this->order(*b); }); }

    /// clear cluster contents for allocated space re-use
    virtual void clear() {
        for(auto o: *this) this->put(o);
        vector<T*>::clear();
    }

    double dx;  ///< clustering interval; modify in derived classes.

    // check whether object time would fall in cluster
    bool inClusterRange(const T& O) { return !this->size() || fabs(order(O) - order(*(this->back()))) <= dx; }

    /// check whether supplied object falls within clustering time of previous event; add to cluster if so.
    bool tryAdd(T* O) {
        if(!O || !inClusterRange(*O)) return false;
        append(O);
        return true;
    }

    /// extract ordering parameter from object
    inline double order(const T& o) const { return double(o); }

protected:
    /// append item to cluster. Override to add more fancy calculations.
    virtual void append(T* o) { this->push_back(o); }

    double x_median = 0;    ///< representative median object position
};

/// OrderedWindow of "clustered" objects
template<class C>
class ClusteredWindow: public OrderedWindow<C> {
public:
    typedef typename C::T T;

    /// Constructor
    ClusteredWindow(double cdx, double dw): OrderedWindow<C>(dw), cluster_dx(cdx), currentC(_get()) { }
    /// Destructor
    virtual ~ClusteredWindow() { this->put(currentC); }

    /// clear remaining objects through window
    void clearWindow() override {
        completeCluster();
        OrderedWindow<C>::clearWindow();
    }

    /// get allocated object space
    T* getSingle() { return currentC->get(); }

    /// add object to newest cluster (or start newer cluster), assuming responsibility for deletion
    virtual void addSingle(T* o) {
        processSingle(*o);
        if(!currentC->tryAdd(o)) {
            completeCluster();
            if(!currentC->tryAdd(o)) throw;
        }
    }

    /// add copy of object
    void addSingle(const T& o) {
        auto O = getSingle();
        *O = o;
        addSingle(O);
    }

    double cluster_dx;  ///< time spread for cluster identification

protected:

    /// process new single element before adding to clustering sequence --- Subclass me!
    virtual void processSingle(T&) { }
    /// decide whether to include cluster
    virtual bool includeCluster(const C&) const { return true; }

    C* currentC;  ///< cluster currently being built

    /// get clustering object with correct cluster time
    C* _get() { auto c = this->get(); c->dx = cluster_dx; return c; }

    /// push currentC into window
    virtual void completeCluster() {
        currentC->close();
        if(includeCluster(*currentC)) {
            this->addItem(currentC);
            currentC = _get();
        } else currentC->clear();
    }
};

#endif
