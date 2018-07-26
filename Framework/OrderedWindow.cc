/// \file OrderedWindow.cc

#include "OrderedWindow.hh"
#include <algorithm> // for std::lower_bound

void OrderedWindowBase::display() const {
    printf("Window of width %g containing %zu events (mid at %zu).\n", hwidth, size(), imid);
    if(verbose > 1) {
        for(size_t i=0; i<size(); i++) {
            if(i==imid) printf("*");
            printf("%g\t", order((*this)[i]));
        }
        printf("\n");
    }
}

void OrderedWindowBase::flushHi(double x) {
    if(verbose >= 2) printf("Flushing window to position %g.\n", x);
    while(size() && order((*this)[imid]) + hwidth <= x) nextmid();
}

void OrderedWindowBase::flushLo(double x) {
    while(size() && order(front()) < x) {
        if(!imid) nextmid();
        else disposeLo();
    }
}

void OrderedWindowBase::_addItem(void* o) {
    if(verbose >= 4) { printf("Adding new "); _display(o); }
    auto x = order(o);
    if(!(x==x)) {
        printf("*** NaN warning at item %i! Skipping!\n ***", nProcessed);
        _display(o);
        dispose(o);
    } else {
        flushHi(x);
        push_back(o);
        _processNew(o);
    }
    nProcessed++;
}

void OrderedWindowBase::nextmid() {
    assert(imid < size());
    if(verbose >= 4) { printf("Transferring mid %zu/%zu ", imid, size()); dispObj((*this)[imid]); }
    _processMid((*this)[imid]);
    imid++;

    if(imid < size()) {
        auto x0 = order((*this)[imid]);
        while(order(front()) + hwidth <= x0) disposeLo();
    } else {
        while(size()) disposeLo();
        assert(!imid);
    }
}

void OrderedWindowBase::disposeLo() {
    assert(imid); // never dispose of mid!
    auto o = front();
    _processOld(o);
    if(verbose >= 4) { printf("Deleting old "); dispObj(o); }
    pop_front();
    imid--;       // move imid to continue pointing to same object
    dispose(o);
}

void OrderedWindowBase::clearWindow() {
    if(verbose >= 2) printf("Flushing window with %zu items (mid at %zu).\n", size(), imid);
    while(size()) nextmid();
}

deque<void*>::iterator OrderedWindowBase::_window_position(double x) {
    return std::lower_bound(begin(), end(), x, [this](const void* a, double t) { return order(a) < t; });
}

deque<void*>::const_iterator OrderedWindowBase::_window_position(double x) const {
    return std::lower_bound(begin(), end(), x, [this](const void* a, double t) { return order(a) < t; });
}
