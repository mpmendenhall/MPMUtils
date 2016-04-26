/// \file RefPool.cc
// This file was produced under the employ of the United States Government,
// and is consequently in the PUBLIC DOMAIN, free from all provisions of
// US Copyright Law (per USC Title 17, Section 105).
//
// -- Michael P. Mendenhall, 2016

#include "RefPool.hh"

void RefPoolItem::release() {
    assert(nrefs);
    nrefs--;
    if(!nrefs) {
        if(myPool) myPool->returnItem(this);
        else delete this;
    }
}

RefPoolItem* RefPool::checkout() {
    if(!items.size()) items.push_back(newItem());
    RefPoolItem* i = items.back();
    items.pop_back();
    i->clear();
    i->retain();
    return i;
}
