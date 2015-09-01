/// \file PluginSaver.cc
// This file was produced under the employ of the United States Government,
// and is consequently in the PUBLIC DOMAIN, free from all provisions of
// US Copyright Law (per USC Title 17, Section 105).
// 
// -- Michael P. Mendenhall, 2015

#include "PluginSaver.hh"
#include "StringManip.hh"
#include <cassert>

PluginSaver::PluginSaver(OutputManager* pnt, const string& nm, const string& inflName): SegmentSaver(pnt, nm, inflName) {
    filePlugins = registerAttrString("filePlugins", "");
    assert(filePlugins);
}

PluginSaver::~PluginSaver() {
    for(auto it = myBuilders.begin(); it != myBuilders.end(); it++) 
        if(it->second->thePlugin) delete it->second->thePlugin;
}

void PluginSaver::buildPlugins() {
    if(fIn) {
        /// try to load all plugins named in input file
        printf("Loading plugins '%s'\n", filePlugins->String().Data());
        auto pnames = split(filePlugins->String().Data(), ",");
        for(auto it = pnames.begin(); it != pnames.end(); it++) {
            auto PB = myBuilders.find(*it);
            if(PB==myBuilders.end()) {
                printf("Plugin '%s' missing from input file; skipped.\n", it->c_str());
                continue;
            }
            PB->second->makePlugin(parent, inflname);
        }
    } else {
        /// construct all plugins
        vector<string> pnames;
        for(auto it = myBuilders.begin(); it != myBuilders.end(); it++) {
            assert(it->second);
            it->second->makePlugin(parent);
            if(it->second->thePlugin) pnames.push_back(it->first);
            else assert(false);
        }
        assert(filePlugins);
        string pstr = join(pnames,",");
        printf("Set up plugins '%s'\n", pstr.c_str());
        filePlugins->SetString(pstr.c_str());
    }
}

SegmentSaver* PluginSaver::getPlugin(const string& nm) const {
    auto PB = myBuilders.find(nm);
    if(PB==myBuilders.end()) return NULL;
    return PB->second->thePlugin;
}

void PluginSaver::zeroSavedHists() {
    SegmentSaver::zeroSavedHists();
    for(auto it = myBuilders.begin(); it != myBuilders.end(); it++)
        if(it->second->thePlugin) it->second->thePlugin->zeroSavedHists();
}

void PluginSaver::scaleData(double s) {
    SegmentSaver::scaleData(s);
    for(auto it = myBuilders.begin(); it != myBuilders.end(); it++)
        if(it->second->thePlugin) it->second->thePlugin->scaleData(s);
}

void PluginSaver::normalize() {
    SegmentSaver::normalize();
    for(auto it = myBuilders.begin(); it != myBuilders.end(); it++)
        if(it->second->thePlugin) it->second->thePlugin->normalize();
}

void PluginSaver::addSegment(const SegmentSaver& S) {
    const PluginSaver& PS = dynamic_cast<const PluginSaver&>(S);
    for(auto it = myBuilders.begin(); it != myBuilders.end(); it++) {
        if(it->second->thePlugin) {
            SegmentSaver* Si = PS.getPlugin(it->first);
            if(Si) it->second->thePlugin->addSegment(*Si);
        }
    }
}

void PluginSaver::makePlots() {
    SegmentSaver::makePlots();
    for(auto it = myBuilders.begin(); it != myBuilders.end(); it++) {
        defaultCanvas->SetLogz(false);
        defaultCanvas->SetLogx(false);
        defaultCanvas->SetLogy(false);
        if(it->second->thePlugin) it->second->thePlugin->makePlots();
    }
}

void PluginSaver::compare(const vector<SegmentSaver*>& v) {
    SegmentSaver::compare(v);
    
    vector<PluginSaver*> vP;
    for(auto it = v.begin(); it != v.end(); it++) vP.push_back(dynamic_cast<PluginSaver*>(*it));
    
    for(auto it = myBuilders.begin(); it != myBuilders.end(); it++) {
        if(!it->second->thePlugin) continue;
        vector<SegmentSaver*> vPi;
        for(auto it2 = vP.begin(); it2 != vP.end(); it2++) {
            if(!(*it2)) vPi.push_back(NULL);
            else vPi.push_back((*it2)->getPlugin(it->first));
        }
        it->second->thePlugin->compare(vPi);
    }
}

void PluginSaver::calculateResults() {
    SegmentSaver::calculateResults();
    for(auto it = myBuilders.begin(); it != myBuilders.end(); it++)
        if(it->second->thePlugin) it->second->thePlugin->calculateResults();
}

void PluginSaver::writeItems() {
    SegmentSaver::writeItems();
    printf("Writing plugins '%s'\n", filePlugins->String().Data());
    for(auto it = myBuilders.begin(); it != myBuilders.end(); it++)
        if(it->second->thePlugin) it->second->thePlugin->writeItems();
}

void PluginSaver::clearItems() {
    SegmentSaver::clearItems();
    for(auto it = myBuilders.begin(); it != myBuilders.end(); it++)
        if(it->second->thePlugin) it->second->thePlugin->clearItems();
}
