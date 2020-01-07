/// \file TObjCollector.cc
// This file was produced under the employ of the United States Government,
// and is consequently in the PUBLIC DOMAIN, free from all provisions of
// US Copyright Law (per USC Title 17, Section 105).
//
// -- Michael P. Mendenhall, 2015

#include "TObjCollector.hh"
#include <stdexcept>

TDirectory* TObjCollector::writeItems(TDirectory* d) {
    if(!d) d = gDirectory;
    d->cd();
    for(auto i: namedItems) i->Write();
    for(auto const& kv: anonItems) kv.second->Write(kv.first.c_str());
    return d;
}

void TObjCollector::clearItems() {
    for(auto i: namedItems) delete i;
    namedItems.clear();
    for(auto const& kv: anonItems) delete kv.second;
    anonItems.clear();
    for(auto i: deleteItems) delete i;
    deleteItems.clear();
}

TNamed* TObjCollector::addObject(TNamed* o) {
    namedItems.push_back(o);
    return o;
}

TH1F* TObjCollector::registeredTH1F(string hname, string htitle, unsigned int nbins, float x0, float x1) {
    TH1F* h = new TH1F(hname.c_str(), htitle.c_str(), nbins, x0, x1);
    return (TH1F*)addObject(h);
}

TH1D* TObjCollector::registeredTH1D(string hname, string htitle, unsigned int nbins, float x0, float x1) {
    TH1D* h = new TH1D(hname.c_str(), htitle.c_str(), nbins, x0, x1);
    return (TH1D*)addObject(h);
}

TH2F* TObjCollector::registeredTH2F(string hname, string htitle, unsigned int nbinsx, float x0, float x1, unsigned int nbinsy, float y0, float y1) {
    return (TH2F*)addObject(new TH2F(hname.c_str(), htitle.c_str(), nbinsx, x0, x1, nbinsy, y0, y1));
}

TObject* TObjCollector::addDeletable(TObject* o) {
    deleteItems.push_back(o);
    return o;
}

TObject* TObjCollector::addWithName(TObject* o, const string& name) {
    if(anonItems.count(name)) throw std::runtime_error("Duplicate name '"+name+"' registered!");
    anonItems[name] = o;
    return o;
}
