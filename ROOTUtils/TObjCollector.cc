/// @file TObjCollector.cc
// This file was produced under the employ of the United States Government,
// and is consequently in the PUBLIC DOMAIN, free from all provisions of
// US Copyright Law (per USC Title 17, Section 105).
//
// -- Michael P. Mendenhall, 2015

#include "TObjCollector.hh"
#include "StringManip.hh"
#include <TH1.h>
#include <stdexcept>

TDirectory* TObjCollector::writeItems(TDirectory* d) {
    if(!d) d = gDirectory;

    for(auto const& kv: namedItems) {
        auto pp = splitLast(kv.first, "/");
        if(pp.first.size()) {
            if(!d->GetDirectory(pp.first.c_str())) d->mkdir(pp.first.c_str());
            d->cd(pp.first.c_str());
        } else d->cd();
        kv.second->Write(pp.second.c_str());
    }

    d->cd();
    return d;
}

void TObjCollector::_addObject(TNamed* o) {
    if(!o) return;
    string n(o->GetName());
    _addObject(o, n);
}

void TObjCollector::_addObject(TObject* o, const string& name) {
    if(!o) return;
    if(namedItems.count(name)) throw std::runtime_error("Duplicate name '"+name+"' registered!");
    if(!name.size()) throw std::runtime_error("Adding item with empty name");
    namedItems[name] = o;
}

void TObjCollector::deleteAll() {
    for(auto const& kv: namedItems) delete kv.second;
    namedItems.clear();
    for(auto i: deleteItems) delete i;
    deleteItems.clear();
}

TObject* TObjCollector::addDeletable(TObject* o) {
    auto h = dynamic_cast<TH1*>(o);
    if(h) h->SetDirectory(nullptr);
    deleteItems.push_back(o);
    return o;
}

