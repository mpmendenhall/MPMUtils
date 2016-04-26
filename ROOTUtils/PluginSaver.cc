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
    for(auto& kv: myBuilders)
        if(kv.second->thePlugin)
            delete kv.second->thePlugin;
}

void PluginSaver::buildPlugins() {
    if(fIn) {
        /// try to load all plugins named in input file
        printf("Loading plugins '%s'\n", filePlugins->String().Data());
        for(auto& pnm: split(filePlugins->String().Data(), ",")) {
            auto PB = myBuilders.find(pnm);
            if(PB==myBuilders.end()) {
                printf("Plugin '%s' missing from input file; skipped.\n", pnm.c_str());
                continue;
            }
            PB->second->makePlugin(this);
        }
    } else {
        /// construct all plugins
        vector<string> pnames;
        for(auto& kv: myBuilders) {
            assert(kv.second);
            kv.second->makePlugin(this);
            if(kv.second->thePlugin) pnames.push_back(kv.first);
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
    if(PB==myBuilders.end()) return nullptr;
    return PB->second->thePlugin;
}

void PluginSaver::zeroSavedHists() {
    SegmentSaver::zeroSavedHists();
    for(auto& kv: myBuilders)
        if(kv.second->thePlugin)
            kv.second->thePlugin->zeroSavedHists();
}

void PluginSaver::scaleData(double s) {
    SegmentSaver::scaleData(s);
    for(auto& kv: myBuilders)
        if(kv.second->thePlugin)
            kv.second->thePlugin->scaleData(s);
}

void PluginSaver::normalize() {
    _normalize();
    for(auto& kv: myBuilders)
        if(kv.second->thePlugin)
            kv.second->thePlugin->normalize();
}

void PluginSaver::addSegment(const SegmentSaver& S, double sc) {
    SegmentSaver::addSegment(S);
    const PluginSaver& PS = dynamic_cast<const PluginSaver&>(S);
    for(auto& kv: myBuilders) {
        if(kv.second->thePlugin) {
            SegmentSaver* Si = PS.getPlugin(kv.first);
            if(Si) kv.second->thePlugin->addSegment(*Si,sc);
            else printf("Warning: PluginSaver::addSegment missing matching plugin for '%s'\n", kv.first.c_str());
        }
    }
}

void PluginSaver::makePlots() {
    defaultCanvas.cd();
    SegmentSaver::makePlots(); 
    for(auto& kv: myBuilders) {
        if(kv.second->thePlugin) {
            kv.second->thePlugin->defaultCanvas.cd();
            kv.second->thePlugin->makePlots();
        }
    }
}

void PluginSaver::finishData() {
    for(auto& kv: myBuilders)
        if(kv.second->thePlugin)
            kv.second->thePlugin->finishData();
}

void PluginSaver::compare(const vector<SegmentSaver*>& v) {
    SegmentSaver::compare(v);
    
    vector<PluginSaver*> vP;
    for(auto SS: v) vP.push_back(dynamic_cast<PluginSaver*>(SS));
    
    for(auto& kv: myBuilders) {
        if(!kv.second->thePlugin) continue;
        vector<SegmentSaver*> vPi;
        for(auto PS: vP) {
            if(!PS) vPi.push_back(nullptr);
            else vPi.push_back(PS->getPlugin(kv.first));
        }
        kv.second->thePlugin->defaultCanvas.cd();
        kv.second->thePlugin->compare(vPi);
    }
}

void PluginSaver::calculateResults() {
    SegmentSaver::calculateResults();
    for(auto& kv: myBuilders)
        if(kv.second->thePlugin)
            kv.second->thePlugin->calculateResults();
}

TDirectory* PluginSaver::writeItems(TDirectory* d) {
    d = SegmentSaver::writeItems(d);
    printf("Writing plugins '%s'\n", filePlugins->String().Data());
    for(auto& kv: myBuilders)
        if(kv.second->thePlugin)
            kv.second->thePlugin->writeItems(d);
    return d;
}

void PluginSaver::clearItems() {
    SegmentSaver::clearItems();
    for(auto& kv: myBuilders)
        if(kv.second->thePlugin)
            kv.second->thePlugin->clearItems();
}
