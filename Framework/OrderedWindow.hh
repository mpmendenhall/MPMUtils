/// \file OrderedWindow.hh Base class for "window" ordered items flow-through analysis
// -- Michael P. Mendenhall, LLNL 2020

#ifndef ORDEREDWINDOW_HH
#define ORDEREDWINDOW_HH

#include "DataSink.hh"
#include "SFINAEFuncs.hh" // for dispObj

#include <cmath>        // for std::fabs
#include <algorithm>    // for std::lower_bound
#include <type_traits>  // for std::remove_pointer
#include <iterator>     // for std::distance
#include <utility>      // for std::pair
#include <stdexcept>
#include <deque>
using std::deque;

/// `for(auto& x: ItRange(start, end))`
template<class iterator>
class ItRange: public std::pair<iterator, const iterator> {
public:
    /// Constructor
    ItRange(const iterator& i0, const iterator& i1): std::pair<iterator, const iterator>(i0,i1) { }
    /// range start
    iterator& begin() { return this->first; }
    /// range end
    const iterator& end() const { return this->second; }
    /// number of elements
    size_t size() const { return this->first==this->second? 0 : std::distance(this->first, this->second); }
};

/// Flow-through analysis on a ``window'' of ordered objects
/// input is always const T; inspection functions depend on const-ness of T
template<class T, typename _ordering_t = typename std::remove_pointer<T>::type::ordering_t>
class OrderedWindow: protected deque<typename std::remove_const<T>::type>, public DataSink<const T> {
public:
    /// internal mutable type
    typedef typename std::remove_const<T>::type Tmut_t;
    /// ordering type
    typedef _ordering_t ordering_t;
    /// internal queue type
    typedef deque<Tmut_t> deque_t;
    /// iterator type
    typedef typename deque_t::iterator iterator;
    /// iterator range
    typedef ItRange<iterator> itrange_t;
    /// const_iterator type
    typedef typename deque_t::const_iterator const_iterator;
    /// const_iterator range
    typedef ItRange<const_iterator> const_itrange_t;

    /// get ordering parameter for object
    template<typename U>
    static ordering_t order(const U& o) { return ordering_t(o); }
    /// get ordering parameter for object
    template<typename U>
    static ordering_t order(const U* o) { return ordering_t(*o); }

    /// Constructor
    explicit OrderedWindow(ordering_t dw): hwidth(dw) { }
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
    void signal(datastream_signal_t sig) override {
        if(sig < DATASTREAM_FLUSH) return;
        if(size()) {
            window_Hi = order(back());
            window_Lo = window_Hi - 2*hwidth;
        }
        while(size()) nextmid();
    }
    /// Flush as if inserting new highest at x
    void flushHi(ordering_t x) {
        window_Hi = x;
        if(!size()) window_Lo = x - 2*hwidth;
        while(size() && order((*this)[imid]) + hwidth <= x) nextmid();
    }
    /// Flush until lowest > x (or queue empty)
    void flushLo(ordering_t x) {
        window_Hi = std::max(window_Hi, x + 2*hwidth);
        while(size() && order(front()) <= x) {
            if(!imid) nextmid();
            else disposeLo();
        }
    }

    /// number of objects in window
    using deque_t::size;
    using deque_t::front;
    using deque_t::back;

    /// get current middle element
    const T& getMid() const { return this->at(imid); }
    /// get ordering position of middle object
    ordering_t xMid() const { if(!size()) return {}; return order(this->at(imid)); }

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
        if(!(dx0 <= dx1)) throw std::runtime_error("Invalid reverse-order rel_range requested");
        if(std::fabs(dx0) > hwidth || std::fabs(dx1) > hwidth) throw std::runtime_error("Rel_range larger than window requested");
        return _rel_range(dx0,dx1);
    }

    /// get (const) window position range for range offset from mid -- no bounds check
    const_itrange_t _rel_range(ordering_t dx0, ordering_t dx1) const { return {rel_position(dx0), rel_position(dx1)}; }
    /// get (const) window position range for range offset from mid
    const_itrange_t rel_range(ordering_t dx0, ordering_t dx1) const {
        if(!size()) throw std::runtime_error("rel_range undefined on empty window");
        if(!(dx0 <= dx1)) throw std::runtime_error("Invalid reverse-order rel_range requested");
        if(std::fabs(dx0) > hwidth || std::fabs(dx1) > hwidth) throw std::runtime_error("rel_range larger than window requested");
        return _rel_range(dx0,dx1);
    }

    /// count items in relative range
    size_t rel_count(ordering_t dx0, ordering_t dx1) const { return rel_range(dx0,dx1).size(); }

    bool enforceClear = true;                       ///< fail if window not clear on destruction
    bool enforceBounds = false;                     ///< local paranoid bounds checking

    /// get window position range for absolute range (no bounds checking)
    itrange_t _abs_range(ordering_t x0, ordering_t x1) { return {abs_position(x0), abs_position(x1)}; }
    /// get window position range for absolute range
    itrange_t abs_range(ordering_t x0, ordering_t x1) {
        if(!(x0 <= x1)) throw std::runtime_error("Invalid reverse-order abs_range requested");
        if(enforceBounds && (!in_range(x0) || !in_range(x1)))
            throw std::runtime_error("abs_range outside window requested");
        return _abs_range(x0,x1);
    }

    /// get (const) window position range for absolute range
    const_itrange_t _abs_range(ordering_t x0, ordering_t x1) const { return {abs_position(x0), abs_position(x1)}; }
    /// get (const) window position range for absolute range
    const_itrange_t abs_range(ordering_t x0, ordering_t x1) const {
        if(!(x0 <= x1)) throw std::runtime_error("Invalid reverse-order abs_range requested");
        if(enforceBounds && (!in_range(x0) || !in_range(x1)))
            throw std::runtime_error("abs_range outside window requested");
        return _abs_range(x0,x1);
    }

    /// count items in absolute range
    size_t abs_count(ordering_t dx0, ordering_t dx1) const { return abs_range(dx0,dx1).size(); }

    /// add next newer object; process older as they pass through window.
    void push(const T& o) override {
        if(verbose >= 4) { printf("Adding new "); display(o); }

        auto x = order(o);
        if(!(x==x)) {
            printf("*** NaN warning at item %i! Skipping!\n ***", nProcessed);
            display(o);
        } else if(hwidth && x < window_Lo && size()) {
            processDisordered(o);
        } else {
            if(!hwidth) while(size()) nextmid();
            else flushHi(x);
            deque_t::push_back(o);
            processNew(back());
        }

        ++nProcessed;
    }

    ordering_t window_Lo = {};  ///< newest discarded (start of available range)
    ordering_t window_Hi = {};  ///< newest added/flushed (end of available range)

protected:

    ordering_t hwidth;  ///< half-length of analysis window kept around "mid" object

    using deque_t::begin;
    using deque_t::end;
    using deque_t::pop_front;

    // Subclass me to do the interesting stuff!

    /// handle acceptance of out-of-order items
    virtual void processDisordered(const T& o) {
        printf("Out-of-order (< %g (%g)) window entry: ", window_Hi, hwidth);
        display(o);
        throw std::runtime_error("Disordered window event");
    }
    /// processing hook for each object as it first enters window
    virtual void processNew(T&) { }
    /// processing hook for each object as it passes through middle of window
    virtual void processMid(T&) { }
    /// processing hook for objects leaving the window
    virtual void processOld(T& o) { if(verbose >= 4) { printf("Removing old "); display(o); } }
    /// display object
    virtual void display(const T& o) const { dispObj(o); }

    size_t imid = 0;    ///< index of "middle" object; always valid if size() > 0

    /// analyze current "mid" object with processMid() and increment to next; flush if no next available
    void nextmid() {
        processMid(this->at(imid));
        window_Lo = order((*this)[imid]) - hwidth;
        ++imid;

        if(imid < size()) {
            while(size() && order(front()) <= window_Lo) disposeLo();
        } else while(size()) disposeLo();
    }

    /// delete oldest object off "older" queue; decrement imid to point to same item
    void disposeLo() {
        processOld(front());
        pop_front();
        --imid; // move imid to continue pointing to same object
    }
};

#endif
