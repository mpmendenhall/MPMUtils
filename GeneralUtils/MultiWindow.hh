/// \file MultiWindow.hh Manager for multiple simulatneous window analyses
// Michael P. Mendenhall, 2018

#ifndef MULTIWINDOW_HH
#define MUTLIWINDOW_HH

#include "OrderedWindow.hh"
#include "AllocPool.hh"
#include <vector>
using std::vector;
#include <cfloat>

/// container for object belonging to a sub-window of a MultiWindow
struct subwindowObj {
    void* o;                ///< the object
    OrderedWindowBase* W;   ///< subwindow for this object type
    /// for compatibility with OrderedWindow
    void clear() { o = nullptr; W = nullptr; }
    /// ordering parameter, for compatibility with OrderedWindow
    operator double() { assert(W && o); return W->order(o); }
};

/// Manager for multiple simulatneous window analyses
class MultiWindow: protected OrderedWindow<subwindowObj>, protected AllocPool<subwindowObj> {
public:
    using OrderedWindow<subwindowObj>::OrderedWindow;
    using OrderedWindow<subwindowObj>::clearWindow;
    using OrderedWindow<subwindowObj>::verbose;
    using OrderedWindow<subwindowObj>::display;

    /// add next newer object to specified window
    void addItem(void* o, OrderedWindowBase& W);

protected:
    /// disposal/deletion for objects outside window
    void dispose(void* o) override;
    /// display object
    void _display(void* o) const override { if(o) { auto so = (subwindowObj*)o; so->W->_display(so->o); } }

    /// delete oldest object off "older" queue, calling dispose(); decrement imid to point to same item
    void disposeLo() override;

    /// processing hook for each object as it first enters window
    void _processNew(void* o) override;
    /// processing hook for each object as it passes through middle of window
    void _processMid(void* o) override;
};

#endif
