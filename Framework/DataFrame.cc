/// \file DataFrame.cc

#include "DataFrame.hh"
#include <cassert>

DataManager::~DataManager() {
    if(nAlloc != pool.size()) {
        printf("Memory leak: allocated %zu DataFrames, but %zu returned to pool\n", nAlloc, pool.size());
        display();
    }
    for(auto d: dtypes) delete d;
}

DataFrame& DataManager::getFrame() {
    auto F = get();
    if(!pool.size() && !(nAlloc%1024)) printf("Warning: excessive frame allocation (%zu); possible memory leak!\n", nAlloc);
    F->M = this;
    assert(!F->getRefs());
    F->claim();
    return *F;
}

void DataFrame::release() {
    assert(nrefs);
    if(--nrefs) return;

    size_t j=0;
    for(auto p: *this) if(p) M->dtypes[j++]->dispose(p);
    drop = false;
    M->put(this);
}

void FrameSink::displayTimeSummary(int d) const {
    for(int i=0; i<d; i++) printf("\t");
    timeUse.display();
    printf("\t: %s\n", name.c_str());
}
