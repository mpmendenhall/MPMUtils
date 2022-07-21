/// \file TObjCollector.cc
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

    for(auto const& kv: anonItems) {
        auto pp = splitLast(kv.first, "/");
        if(pp.first.size()) {
            if(!d->GetDirectory(pp.first.c_str())) d->mkdir(pp.first.c_str());
            d->cd(pp.first.c_str());
        } else d->cd();
        kv.second->Write(pp.second.c_str());
    }

    d->cd();
    for(auto i: namedItems) i->Write();

    return d;
}

void TObjCollector::deleteAll() {
    for(auto i: namedItems) delete i;
    namedItems.clear();
    for(auto const& kv: anonItems) delete kv.second;
    anonItems.clear();
    for(auto i: deleteItems) delete i;
    deleteItems.clear();
}

TObject* TObjCollector::addDeletable(TObject* o) {
    auto h = dynamic_cast<TH1*>(o);
    if(h) h->SetDirectory(nullptr);
    deleteItems.push_back(o);
    return o;
}

TObject* TObjCollector::addAnonymous(TObject* o, const string& name) {
    if(anonItems.count(name)) throw std::runtime_error("Duplicate name '"+name+"' registered!");
    anonItems[name] = o;
    return o;
}
