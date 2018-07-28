/// \file MultiWindow.cc

#include "MultiWindow.hh"

void MultiWindow::addItem(void* o, OrderedWindowBase& W) {
    assert(o);
    auto so = get();
    so->o = o;
    so->W = &W;

    auto x = W.order(o);
    if(!(x==x)) {
        printf("*** NaN warning at item %i! Skipping!\n ***", nProcessed);
        _display(so);
        put(so);
    } else {
        flushHi(x);
        push_back(so);
        W.push_back(o);
        _processNew(so);
    }
    nProcessed++;
}

void MultiWindow::_processNew(void* o) {
    auto so = (subwindowObj*)o;
    so->W->_processNew(so->o);
}

void MultiWindow::_processMid(void* o) {
    auto so = (subwindowObj*)o;
    so->W->_processMid(so->o);
    so->W->imid++;
    processMid(*so);
}

void MultiWindow::disposeLo() {
    assert(imid);
    auto o = front();
    pop_front();
    imid--;
    auto so = (subwindowObj*)o;
    so->W->disposeLo();
    dispose(o);
}
