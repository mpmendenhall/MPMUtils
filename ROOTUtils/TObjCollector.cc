/// \file TObjCollector.cc
// This file was produced under the employ of the United States Government,
// and is consequently in the PUBLIC DOMAIN, free from all provisions of
// US Copyright Law (per USC Title 17, Section 105).
// 
// -- Michael P. Mendenhall, 2015

#include "TObjCollector.hh"
#include <cassert>

void TObjCollector::writeItems() {
    printf("Saving registered objects...");
    fflush(stdout);
    for(auto i = namedItems.begin(); i != namedItems.end(); i++) (*i)->Write();
    for(auto i = anonItems.begin(); i != anonItems.end(); i++) i->second->Write(i->first.c_str());
    printf(" Done.\n");
}

void TObjCollector::clearItems() {
    for(auto i = namedItems.begin(); i != namedItems.end(); i++) delete(*i);
    namedItems.clear();
    for(auto i = anonItems.begin(); i != anonItems.end(); i++) delete(i->second);
    anonItems.clear();
    for(auto i = deleteItems.begin(); i != deleteItems.end(); i++) delete(*i);
    deleteItems.clear();
}

TNamed* TObjCollector::addObject(TNamed* o) { 
    namedItems.push_back(o); 
    return o;
}

TObject* TObjCollector::addDeletable(TObject* o) { 
    deleteItems.push_back(o); 
    return o;
}

TObject* TObjCollector::addWithName(TObject* o, const string& name) { 
    assert(!anonItems.count(name)); 
    anonItems[name] = o; 
    return o;
}
