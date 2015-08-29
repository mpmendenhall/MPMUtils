/// \file PluginSaver.cc
// This file was produced under the employ of the United States Government,
// and is consequently in the PUBLIC DOMAIN, free from all provisions of
// US Copyright Law (per USC Title 17, Section 105).
// 
// -- Michael P. Mendenhall, 2015

#include "PluginSaver.hh"
#include "StringManip.hh"

PluginSaver::PluginSaver(OutputManager* pnt, const string& nm, const string& inflName): SegmentSaver(pnt, nm, inflName) {
    filePlugins = registerAttrString("filePlugins", "");
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
            it->second->makePlugin(parent);
            if(it->second->thePlugin) pnames.push_back(it->first);
        }
        filePlugins->SetString(join(pnames,",").c_str());
    }
}

const SegmentSaver* PluginSaver::getPlugin(const string& nm) const {
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
            const SegmentSaver* Si = PS.getPlugin(it->first);
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
