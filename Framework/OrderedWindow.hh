/// \file OrderedWindow.hh Base class for "window" ordered items flow-through analysis
// Michael P. Mendenhall, 2019

#ifndef ORDEREDWINDOW_HH
#define ORDEREDWINDOW_HH

#include "SFINAEFuncs.hh" // for dispObj
#include "DataSink.hh"
#include <cassert>
#include <cmath>
#include <algorithm> // for std::lower_bound
#include <type_traits> // for std::remove_pointer
#include <iterator> // for std::distance
#include <stdexcept>
#include <deque>
using std::deque;
#include <utility>
using std::pair;

/// `for(auto& x: ItRange(start, end))`
template<class iterator>
class ItRange: public pair<iterator, const iterator> {
public:
    /// Constructor
    ItRange(const iterator& i0, const iterator& i1): pair<iterator, const iterator>(i0,i1) { }
    /// range start
    iterator& begin() { return this->first; }
    /// range end
    const iterator& end() const { return this->second; }
    /// number of elements
    size_t size() const { return this->first==this->second? 0 : std::distance(this->first, this->second); }
};

/// Flow-through analysis on a ``window'' of ordered objects
template<class T, typename _ordering_t = double>
class OrderedWindow: protected deque<T>, public DataSink<T> {
public:
    /// ordering type
    typedef _ordering_t ordering_t;
    /// iterator type
    typedef typename deque<T>::iterator iterator;
    /// iterator range
    typedef ItRange<iterator> itrange_t;
    /// const_iterator type
    typedef typename deque<T>::const_iterator const_iterator;
    /// const_iterator range
    typedef ItRange<const_iterator> const_itrange_t;

    /// get ordering parameter for object
    template<typename U>
    static ordering_t order(const U& o) { return ordering_t(o); }
    /// get ordering parameter for object
    template<typename U>
    static ordering_t order(const U* o) { return ordering_t(*o); }

    /// Constructor
    OrderedWindow(ordering_t dw): hwidth(dw) { }
    /// Destructor: must be cleared before reaching here!
    virtual ~OrderedWindow() {
        if(size()) {
            printf("Potential memory leak: unflushed window of %zu objects.\n", size());
            if(enforceClear) abort();
        }
    }

    /// get window half-width
    ordering_t windowHalfwidth() const { return hwidth; }

    /// clear remaining objects through window (at end of run, etc.)
    void flush() override { if(size()) flushHi(order(back()) + hwidth); }
    /// Flush as if inserting new highest at x
    void flushHi(ordering_t x) {
        window_Hi = x;
        if(!size()) window_Lo = x - 2*hwidth;
        while(size() && order((*this)[imid]) + hwidth <= x) nextmid();
    }
    /// Flush until lowest > x (or queue empty)
    void flushLo(ordering_t x) {
        window_Hi = std::max(window_Hi, x + 2*hwidth);
        while(size() && order(front()) < x) {
            if(!imid) nextmid();
            else disposeLo();
        }
    }

    /// number of objects in window
    using deque<T>::size;
    using deque<T>::front;
    using deque<T>::back;

    /// get current middle element
    const T& getMid() const { return this->at(imid); }
    /// get ordering position of middle object
    ordering_t xMid() const { if(!size()) return {}; assert(imid < size()); return order((*this)[imid]); }

    /// print window information
    virtual void display() const {
        printf("Window of width %g containing %zu events (mid at %zu).\n", hwidth, size(), imid);
        if(verbose > 1) {
            for(size_t i=0; i<size(); i++) {
                if(i == imid) printf("*");
                printf("%g\t", order((*this)[i]));
            }
            printf("\n");
        }
    }

    int verbose = 0;    ///< verbose level
    int nProcessed = 0; ///< number of objects processed through window

    /// get iterator to first item in window with order >= x
    iterator abs_position(ordering_t x) { return std::lower_bound(begin(), end(), x, [](const T& a, ordering_t t) { return order(a) < t; }); }
    /// get const_iterator to first item in window with order >= x
    const_iterator abs_position(ordering_t x) const { return std::lower_bound(begin(), end(), x, [](const T& a, ordering_t t) { return order(a) < t; }); }
    /// check if item is in available range
    bool in_range(ordering_t x) const { return window_Lo < x && x < window_Hi; }

    /// get iterator to first item in window with order >= xMid + dx
    iterator rel_position(ordering_t dx) { return abs_position(xMid()+dx); }
    /// get const_iterator to first item in window with order >= xMid + dx
    const_iterator rel_position(ordering_t dx) const { return abs_position(xMid()+dx); }

    /// get window position range for range offset from mid --- no bounds check
    itrange_t _rel_range(ordering_t dx0, ordering_t dx1) { return {rel_position(dx0), rel_position(dx1)}; }
    /// get window position range for range offset from mid
    itrange_t rel_range(ordering_t dx0, ordering_t dx1) {
        if(!size()) throw std::runtime_error("Rel_range undefined on empty window");
        if(std::fabs(dx0) > hwidth || std::fabs(dx1) > hwidth) throw std::runtime_error("Rel_range larger than window requested");
        return _rel_range(dx0,dx1);
    }

    /// get (const) window position range for range offset from mid -- no bounds check
    const_itrange_t _rel_range(ordering_t dx0, ordering_t dx1) const { return {rel_position(dx0), rel_position(dx1)}; }
    /// get (const) window position range for range offset from mid
    const_itrange_t rel_range(ordering_t dx0, ordering_t dx1) const {
        if(!size()) throw std::runtime_error("rel_range undefined on empty window");
        if(std::fabs(dx0) > hwidth || std::fabs(dx1) > hwidth) throw std::runtime_error("rel_range larger than window requested");
        return _rel_range(dx0,dx1);
    }

    /// count items in relative range
    size_t rel_count(ordering_t dx0, ordering_t dx1) const { return rel_range(dx0,dx1).size(); }

    static constexpr bool PARANOID_BOUNDS = false;
    bool enforceClear = true;

    /// get window position range for absolute range (no bounds checking)
    itrange_t _abs_range(ordering_t x0, ordering_t x1) { return {abs_position(x0), abs_position(x1)}; }
    /// get window position range for absolute range
    itrange_t abs_range(ordering_t x0, ordering_t x1) {
        if(PARANOID_BOUNDS && (!in_range(x0) || !in_range(x1)))
            throw std::runtime_error("abs_range outside window requested");
        return _abs_range(x0,x1);
    }

    /// get (const) window position range for absolute range
    const_itrange_t _abs_range(ordering_t x0, ordering_t x1) const { return {abs_position(x0), abs_position(x1)}; }
    /// get (const) window position range for absolute range
    const_itrange_t abs_range(ordering_t x0, ordering_t x1) const {
        if(PARANOID_BOUNDS && (!in_range(x0) || !in_range(x1)))
            throw std::runtime_error("abs_range outside window requested");
        return _abs_range(x0,x1);
    }

    /// count items in absolute range
    size_t abs_count(ordering_t dx0, ordering_t dx1) const { return abs_range(dx0,dx1).size(); }

    /// add next newer object; process older as they pass through window.
    void push(const T& oo) override {
        T o = oo; // copy to permit modification/disposal
        if(verbose >= 4) { printf("Adding new "); display(o); }
        auto x = order(o);
        if(!(x==x)) {
            printf("*** NaN warning at item %i! Skipping!\n ***", nProcessed);
            display(o);
            dispose(o);
        } else {
            flushHi(x);
            processNew(o);
            deque<T>::push_back(o);
        }
        nProcessed++;
    }

    ordering_t window_Lo = {};  ///< newest discarded (start of available range)
    ordering_t window_Hi = {};  ///< newest added/flushed (end of available range)

protected:

    ordering_t hwidth;  ///< half-length of analysis window kept around "mid" object

    using deque<T>::begin;
    using deque<T>::end;
    using deque<T>::pop_front;

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
        window_Lo = order((*this)[imid]) - hwidth;
        ++imid;

        if(imid < size()) {
            while(order(front()) <= window_Lo) disposeLo();
        } else {
            while(size()) disposeLo();
            assert(!imid);
        }
    }

    /// delete oldest object off "older" queue, calling dispose(); decrement imid to point to same item
    void disposeLo() {
        assert(imid); // never dispose of mid!
        auto& o = front();
        processOld(o);
        if(verbose >= 4) { printf("Deleting old "); display(o); }
        pop_front();
        imid--;       // move imid to continue pointing to same object
        dispose(o);
    }
};

#endif
