/// \file OrderedWindow.hh Base class for "window" ordered items flow-through analysis
// Michael P. Mendenhall, 2019

#ifndef ORDEREDWINDOW_HH
#define ORDEREDWINDOW_HH

#include "SFINAEFuncs.hh" // for dispObj
#include <cassert>
#include <algorithm> // for std::lower_bound
#include <type_traits> // for std::remove_pointer
#include <deque>
using std::deque;
#include <utility>
using std::pair;

/// Flow-through analysis on a ``window'' of ordered objects
template<class T0, typename ordering_t = double>
class OrderedWindow: protected deque<T0> {
public:
    /// un-pointered class being ordered
    typedef typename std::remove_pointer<T0>::type T;
    /// iterator type
    typedef typename deque<T0>::iterator iterator;
    /// iterator type
    typedef typename deque<T0>::const_iterator const_iterator;

    /// get reference, dereferencing if object is pointer
    template<typename U>
    static U& deref_if_ptr(U& obj) { return obj; }
    /// get reference, dereferencing if object is pointer
    template<typename U>
    static U& deref_if_ptr(U* obj) { return *obj; }
    /// get ordering parameter for object
    template<typename U>
    static ordering_t order(U o) { return ordering_t(deref_if_ptr(o)); }

    /// Constructor
    OrderedWindow(ordering_t dw): hwidth(dw) { }
    /// Destructor: must be cleared before reaching here!
    virtual ~OrderedWindow() { assert(!size()); assert(!imid); }

    /// clear remaining objects through window (at end of run, etc.)
    virtual void clearWindow() { while(size()) nextmid(); }
    /// Flush as if inserting new highest at x
    void flushHi(ordering_t x) { while(size() && order((*this)[imid]) + hwidth <= x) nextmid(); }
    /// Flush until lowest > x (or queue empty)
    void flushLo(ordering_t x) {  while(size() && order(front()) < x) { if(!imid) nextmid(); else disposeLo(); } }

    /// number of objects in window
    using deque<T0>::size;

    /// get ordering position of middle object
    ordering_t xMid() const { return size()? order((*this)[imid]) : 0; }

    /// print window information
    virtual void display() const {
        printf("Window of width %g containing %zu events (mid at %zu).\n", hwidth, size(), imid);
        if(verbose > 1) {
            for(size_t i=0; i<size(); i++) {
                if(i==imid) printf("*");
                printf("%g\t", order((*this)[i]));
            }
            printf("\n");
        }
    }

    int verbose = 0;    ///< verbose level
    int nProcessed = 0; ///< number of objects processed through window

    /// get iterator to first item in window with order >= x
    iterator abs_position(ordering_t x) { return std::lower_bound(begin(), end(), x, [](const T0& a, ordering_t t) { return order(a) < t; }); }

    /// get iterator to first item in window with order >= x
    const iterator abs_position(double x) const { return std::lower_bound(begin(), end(), x, [](const T0& a, ordering_t t) { return order(a) < t; }); }
    /// get iterator to first item in window with order >= xMid + dx
    iterator rel_position(double dx) { return abs_position(xMid()+dx); }
    /// get window position range for range offset from mid
    pair<iterator, iterator> rel_range(double dx0, double dx1) { return {rel_position(dx0), rel_position(dx1)}; }
    /// count items in range
    size_t rel_count(double dx0, double dx1) { return rel_position(dx1).I - rel_position(dx0).I; }
    /// get window position range for absolute range
    pair<iterator, iterator> abs_range(double x0, double x1) { return {abs_position(x0), abs_position(x1)}; }
    /// get number of objects in specified time range around mid
    int window_counts(ordering_t dx0, ordering_t dx1) const { auto x = xMid(); return abs_position(x+dx1) - abs_position(x+dx0); }

    /// add next newer object; process older as they pass through window.
    void addItem(const T0& oo) {
        auto o = oo;
        if(verbose >= 4) { printf("Adding new "); display(o); }
        auto x = order(o);
        if(!(x==x)) {
            printf("*** NaN warning at item %i! Skipping!\n ***", nProcessed);
            display(o);
            dispose(o);
        } else {
            flushHi(x);
            this->push_back(o);
            processNew(o);
        }
        nProcessed++;
    }

protected:

    ordering_t hwidth;  ///< half-length of analysis window kept around "mid" object

    using deque<T0>::front;
    using deque<T0>::pop_front;
    using deque<T0>::begin;
    using deque<T0>::end;

    // Subclass me to do the interesting stuff!

    /// processing hook for each object as it first enters window
    virtual void processNew(T&) { }
    /// processing hook for each object as it passes through middle of window
    virtual void processMid(T&) { }
    /// processing hook for objects leaving the window
    virtual void processOld(T&) { }
    /// disposal/deletion for objects outside window
    virtual void dispose(T&) { }
    /// display object
    virtual void display(const T& o) const { dispObj(o); }

    size_t imid = 0;    ///< index of "middle" object; always valid if size() > 0

    /// analyze current "mid" object with processMid() and increment to next; flush if no next available
    void nextmid() {
        assert(imid < size());
        processMid((*this)[imid]);
        imid++;

        if(imid < size()) {
            auto x0 = order((*this)[imid]);
            while(order(front()) + hwidth <= x0) disposeLo();
        } else {
            while(size()) disposeLo();
            assert(!imid);
        }
    }

    /// delete oldest object off "older" queue, calling dispose(); decrement imid to point to same item
    void disposeLo() {
        assert(imid); // never dispose of mid!
        auto o = front();
        processOld(o);
        if(verbose >= 4) { printf("Deleting old "); display(o); }
        pop_front();
        imid--;       // move imid to continue pointing to same object
        dispose(o);
    }
};

#endif
