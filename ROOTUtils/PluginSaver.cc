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

shared_ptr<SegmentSaver> PluginSaver::getPlugin(const string& nm) const {
    auto PB = myBuilders.find(nm);
    if(PB == myBuilders.end()) return nullptr;
    return PB->second->thePlugin;
}

void PluginSaver::setPrintSuffix(const string& sfx) {
    SegmentSaver::setPrintSuffix(sfx);
    for(auto& kv: myBuilders)
        if(kv.second->thePlugin)
            kv.second->thePlugin->setPrintSuffix(sfx);
}

void PluginSaver::zeroSavedHists() {
    SegmentSaver::zeroSavedHists();
    for(auto& kv: myBuilders)
        if(kv.second->thePlugin)
            kv.second->thePlugin->zeroSavedHists();
}

void PluginSaver::checkStatus() {
    for(auto& kv: myBuilders) {
        if(kv.second->thePlugin) {
            auto t0 = steady_clock::now();
            kv.second->thePlugin->defaultCanvas.cd();
            kv.second->thePlugin->checkStatus();
            kv.second->thePlugin->tPlot += std::chrono::duration<double>(steady_clock::now()-t0).count();
        }
    }
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
            auto Si = PS.getPlugin(kv.first);
            if(Si) kv.second->thePlugin->addSegment(*Si,sc);
            else printf("Warning: PluginSaver::addSegment missing matching plugin for '%s'\n", kv.first.c_str());
        }
    }
}

void PluginSaver::checkpoint(const SegmentSaver& Sprev) {
    auto& PS = dynamic_cast<const PluginSaver&>(Sprev);
    for(auto& kv: myBuilders) {
        if(kv.second->thePlugin) {
            auto Si = PS.getPlugin(kv.first);
            if(Si) kv.second->thePlugin->checkpoint(*Si);
            else printf("Warning: PluginSaver::checkpoint missing matching plugin for '%s'\n", kv.first.c_str());
        }
    }
}

void PluginSaver::makePlots() {
    defaultCanvas.cd();
    SegmentSaver::makePlots();
    for(auto& kv: myBuilders) {
        if(kv.second->thePlugin) {
            auto t0 = steady_clock::now();
            kv.second->thePlugin->defaultCanvas.cd();
            kv.second->thePlugin->makePlots();
            kv.second->thePlugin->tPlot += std::chrono::duration<double>(steady_clock::now()-t0).count();
        }
    }
}

void PluginSaver::startData() {
    for(auto& kv: myBuilders)
        if(kv.second->thePlugin)
            kv.second->thePlugin->startData();
}

void PluginSaver::processEvent() {
    for(auto& kv: myBuilders)
        if(kv.second->thePlugin)
            kv.second->thePlugin->processEvent();
}

void PluginSaver::finishData() {
    for(auto& kv: myBuilders) {
        if(kv.second->thePlugin) {
            auto t0 = steady_clock::now();
            kv.second->thePlugin->finishData();
            kv.second->thePlugin->tProcess += std::chrono::duration<double>(steady_clock::now()-t0).count();
        }
    }
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
            else vPi.push_back(PS->getPlugin(kv.first).get());
        }
        kv.second->thePlugin->defaultCanvas.cd();
        kv.second->thePlugin->compare(vPi);
    }
}

void PluginSaver::calculateResults() {
    SegmentSaver::calculateResults();
    for(auto& kv: myBuilders) {
        if(kv.second->thePlugin) {
            auto t0 = steady_clock::now();
            kv.second->thePlugin->calculateResults();
            kv.second->thePlugin->tCalc += std::chrono::duration<double>(steady_clock::now()-t0).count();
        }
    }
}

double PluginSaver::displayTimeUse() const {
    printf("\n-------------- Plugin time use\n");
    printf("\tsetup\tprocess\tcalc\tplot\t\ttotal\n");
    double tall = 0;
    double p_tSetup = 0;
    double p_tProcess = 0;
    double p_tCalc = 0;
    double p_tPlot = 0;
    for(auto& kv: myBuilders) {
        auto& pb = kv.second->thePlugin;
        if(pb) {
            double ttot = pb->tSetup + pb->tProcess + pb->tCalc + pb->tPlot;
            printf("* %s\n\t%.2f\t%.2f\t%.2f\t%.2f\t\t%.2f s\n", kv.first.c_str(),
                   pb->tSetup, pb->tProcess, pb->tCalc, pb->tPlot, ttot);
            p_tSetup += pb->tSetup;
            p_tProcess += pb->tProcess;
            p_tCalc += pb->tCalc;
            p_tPlot += pb->tPlot;
            tall += ttot;
        }
    }
    printf("----- Total ------\n\t%.2f\t%.2f\t%.2f\t%.2f\t\t%.2f s\n",
           p_tSetup, p_tProcess, p_tCalc, p_tPlot, tall);
    return tall;
}

TDirectory* PluginSaver::writeItems(TDirectory* d) {
    d = SegmentSaver::writeItems(d);
    if(myBuilders.size()) printf("Writing plugins '%s'\n", filePlugins->String().Data());
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
