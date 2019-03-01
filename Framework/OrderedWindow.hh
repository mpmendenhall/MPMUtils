/// \file OrderedWindow.hh Base class for "window" ordered items flow-through analysis
// Michael P. Mendenhall, 2018

#ifndef ORDEREDWINDOW_HH
#define ORDEREDWINDOW_HH

#include "RecastIt.hh"
#include "SFINAEFuncs.hh"
#include "AllocPool.hh"
#include <deque>
#include <cassert>
#include <stdio.h>
using std::deque;
using std::pair;

class MultiWindow;
struct subwindowObj;

/// Generic inheritance base
class OrderedWindowBase: protected deque<void*> {
public:
    /// Constructor
    OrderedWindowBase(double dw): hwidth(dw) { }
    /// Destructor: must be cleared before reaching here!
    virtual ~OrderedWindowBase() { assert(!size()); assert(!imid); }

    /// clear remaining objects through window (at end of run, etc.)
    virtual void clearWindow();
    /// Flush as if inserting new highest at x
    void flushHi(double x);
    /// Flush until lowest > x (or queue empty)
    void flushLo(double x);

    /// number of objects in window
    using deque<void*>::size;
    using deque<void*>::iterator;
    /// get ordering position of middle object
    double xMid() const { return size()? order((*this)[imid]) : 0; }
    /// get number of objects in specified time range around mid
    int window_counts(double dx0, double dx1) { auto x = xMid(); return _window_position(x+dx1)-_window_position(x+dx0); }
    /// print window information
    virtual void display() const;

    double hwidth;      ///< half-length of analysis window kept around "mid" object
    int verbose = 0;    ///< verbose level
    int nProcessed = 0; ///< number of objects processed through window

protected:
    friend class MultiWindow;
    friend struct subwindowObj;

    /// add next newer object; process older as they pass through window.
    void _addItem(void* o);
    /// get iterator to first item in window with order >= x
    iterator _window_position(double x);
    /// get iterator to first item in window with order >= x
    const_iterator _window_position(double x) const;
    /// processing hook for each object as it first enters window
    virtual void _processNew(void* o) = 0;
    /// processing hook for each object as it passes through middle of window
    virtual void _processMid(void* o) = 0;
    /// processing hook for objects leaving the window
    virtual void _processOld(void* o) = 0;

    /// extract ordering parameter from object
    virtual double order(const void* o) const = 0;
    /// disposal/deletion for objects outside window
    virtual void dispose(void* o) = 0;
    /// display object
    virtual void _display(void* o) const = 0;

    size_type imid = 0; ///< index of "middle" object; always valid if size() > 0

    /// analyze current "mid" object with processMid() and increment to next; flush if no next available
    void nextmid();
    /// delete oldest object off "older" queue, calling dispose(); decrement imid to point to same item
    virtual void disposeLo();
};

/// "Coincidence window" analysis base class
template<class T>
class OrderedWindow: public OrderedWindowBase, protected AllocPool<T> {
public:
    /// Constructor
    using OrderedWindowBase::OrderedWindowBase;

    /// add next newer object; process older as they pass through window.
    void addItem(T* o) { _addItem(o); }
    /// add copy of item
    void addItem(const T& o) {
        auto O = this->get();
        *O = o;
        addItem(O);
    }

    /// convenience typedef
    typedef recastIt<OrderedWindowBase::iterator, T> iterator;
    typedef recastIt<OrderedWindowBase::const_iterator, const T> const_iterator;

    /// get iterator to first item in window with order >= x
    iterator abs_position(double x) { return _window_position(x); }
    /// get iterator to first item in window with order >= x
    const iterator abs_position(double x) const { return _window_position(x); }
    /// get iterator to first item in window with order >= xMid + dx
    iterator rel_position(double dx) { return abs_position(xMid()+dx); }
    /// get window position range for range offset from mid
    pair<iterator, iterator> rel_range(double dx0, double dx1) { return {rel_position(dx0), rel_position(dx1)}; }
    /// count items in range
    size_t rel_count(double dx0, double dx1) { return rel_position(dx1).I - rel_position(dx0).I; }
    /// get window position range for absolute range
    pair<iterator, iterator> abs_range(double x0, double x1) { return {abs_position(x0), abs_position(x1)}; }
    /// start of full range
    iterator begin() { return deque<void*>::begin(); }
    /// start of full range
    iterator end() { return deque<void*>::end(); }
    /// start of full range
    const_iterator begin() const { return deque<void*>::begin(); }
    /// start of full range
    const_iterator end() const { return deque<void*>::end(); }

    /// const access to mid item
    const T* getMid() const { return imid < size()? (T*)(*this)[imid] : nullptr; }

    /// summary display of this window
    void display() const override { OrderedWindowBase::display(); }
    /// extract ordering parameter from object
    double order(const void* o) const override { return double(*(T*)(o)); }

protected:

    // -------------- subclass me! ------------------
    /// process newest object in window
    virtual void processNew(T&) { }
    /// processing hook for each object as it passes through middle of window
    virtual void processMid(T&) { }
    /// processing hook for objects leaving the window
    virtual void processOld(T&) { }

    /// disposal/deletion for objects outside window
    void dispose(void* o) override { this->put((T*)o); }
    /// display object
    void _display(void* o) const override { if(o) dispObj(*(T*)o); }

    // -------------- convenience recasts to above functions ----------------
    /// processing hook for each object as it first enters window
    void _processNew(void* o) override { assert(o); processNew(*(T*)o); }
    /// processing hook for each object as it passes through middle of window
    void _processMid(void* o) override { assert(o); processMid(*(T*)o); }
    /// processing hook for objects leaving the window
    void _processOld(void* o) override { assert(o); processOld(*(T*)o); }
};

#endif
