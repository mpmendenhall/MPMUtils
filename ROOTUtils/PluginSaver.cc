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
            throw nullptr; // TODO
            //myPlugins.push_back(PB->second->construct(this));
        }
    } else {
        /// construct all plugins
        vector<string> pnames;
        for(auto& kv: myBuilders) {
            pnames.push_back(kv.first);
            throw nullptr; // TODO
            //myPlugins.push_back(kv.second->construct(this));
        }
        assert(filePlugins);
        string pstr = join(pnames,",");
        printf("Set up plugins '%s'\n", pstr.c_str());
        filePlugins->SetString(pstr.c_str());
    }
    std::sort(myPlugins.begin(), myPlugins.end(),
              [](SegmentSaver* a, SegmentSaver* b) { return a->order < b->order; });
}

map<string,float> PluginSaver::compareKolmogorov(const SegmentSaver& S) const {
    auto m = SegmentSaver::compareKolmogorov(S);
    auto& PS = dynamic_cast<const PluginSaver&>(S);
    for(auto P: myPlugins) {
        auto Si = PS.getPlugin(P->name);
        if(!Si) continue;
        auto mm = P->compareKolmogorov(*Si);
        for(auto& kv: mm) m[P->name + "." + kv.first] = kv.second;
    }
    return m;
}

shared_ptr<SegmentSaver> PluginSaver::getPlugin(const string& nm) const {
    throw nullptr; // TODO
    //auto PB = myBuilders.find(nm);
    //if(PB == myBuilders.end()) return nullptr;
    //return PB->second->thePlugin;
}

void PluginSaver::setPrintSuffix(const string& sfx) {
    SegmentSaver::setPrintSuffix(sfx);
    for(auto P: myPlugins) P->setPrintSuffix(sfx);
}

void PluginSaver::zeroSavedHists() {
    SegmentSaver::zeroSavedHists();
    for(auto P: myPlugins) P->zeroSavedHists();
}

void PluginSaver::checkStatus() {
    for(auto P: myPlugins) {
        auto t0 = steady_clock::now();
        P->defaultCanvas.cd();
        P->checkStatus();
        P->tPlot += std::chrono::duration<double>(steady_clock::now()-t0).count();
    }
}

void PluginSaver::scaleData(double s) {
    SegmentSaver::scaleData(s);
    for(auto P: myPlugins) P->scaleData(s);
}

void PluginSaver::normalize() {
    _normalize();
    for(auto P: myPlugins) P->normalize();
}

void PluginSaver::addSegment(const SegmentSaver& S, double sc) {
    SegmentSaver::addSegment(S);
    auto& PS = dynamic_cast<const PluginSaver&>(S);
    for(auto P: myPlugins) {
        auto Si = PS.getPlugin(P->name);
        if(Si) P->addSegment(*Si,sc);
        else printf("Warning: PluginSaver::addSegment missing matching plugin for '%s'\n", P->name.c_str());
    }
}

void PluginSaver::checkpoint(const SegmentSaver& Sprev) {
    auto& PS = dynamic_cast<const PluginSaver&>(Sprev);
    for(auto P: myPlugins) {
        auto Si = PS.getPlugin(P->name);
        if(Si) P->checkpoint(*Si);
        else printf("Warning: PluginSaver::checkpoint missing matching plugin for '%s'\n", P->name.c_str());
    }
}

void PluginSaver::makePlots() {
    defaultCanvas.cd();
    SegmentSaver::makePlots();
    for(auto P: myPlugins) {
        auto t0 = steady_clock::now();
        P->defaultCanvas.cd();
        P->makePlots();
        P->tPlot += std::chrono::duration<double>(steady_clock::now()-t0).count();
    }
}

void PluginSaver::startData() {
    for(auto P: myPlugins) {
        auto t0 = steady_clock::now();
        P->startData();
        P->tProcess += std::chrono::duration<double>(steady_clock::now()-t0).count();
    }
}

void PluginSaver::processEvent() {
    for(auto P: myPlugins) P->processEvent();
}

void PluginSaver::finishData(bool f) {
    for(auto P: myPlugins) {
        auto t0 = steady_clock::now();
        P->finishData(f);
        P->tProcess += std::chrono::duration<double>(steady_clock::now()-t0).count();
    }
}

void PluginSaver::compare(const vector<SegmentSaver*>& v) {
    SegmentSaver::compare(v);

    vector<PluginSaver*> vP;
    for(auto SS: v) vP.push_back(dynamic_cast<PluginSaver*>(SS));

    for(auto P: myPlugins) {
        vector<SegmentSaver*> vPi;
        for(auto PS: vP) {
            if(!PS) vPi.push_back(nullptr);
            else vPi.push_back(PS->getPlugin(P->name).get());
        }
        P->defaultCanvas.cd();
        P->compare(vPi);
    }
}

void PluginSaver::calculateResults() {
    SegmentSaver::calculateResults();
    for(auto P: myPlugins) {
        auto t0 = steady_clock::now();
        printf("\n## PLUGIN %s CalculateResults ##\n\n", P->name.c_str());
        P->calculateResults();
        P->tCalc += std::chrono::duration<double>(steady_clock::now()-t0).count();
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
    for(auto pb: myPlugins) {
        double ttot = pb->tSetup + pb->tProcess + pb->tCalc + pb->tPlot;
        printf("* %s\n\t%.2f\t%.2f\t%.2f\t%.2f\t\t%.2f s\n", pb->name.c_str(),
                pb->tSetup, pb->tProcess, pb->tCalc, pb->tPlot, ttot);
        p_tSetup += pb->tSetup;
        p_tProcess += pb->tProcess;
        p_tCalc += pb->tCalc;
        p_tPlot += pb->tPlot;
        tall += ttot;
    }
    printf("----- Total ------\n\t%.2f\t%.2f\t%.2f\t%.2f\t\t%.2f s\n",
           p_tSetup, p_tProcess, p_tCalc, p_tPlot, tall);
    return tall;
}

TDirectory* PluginSaver::writeItems(TDirectory* d) {
    d = SegmentSaver::writeItems(d);
    if(myBuilders.size()) printf("Writing plugins '%s'\n", filePlugins->String().Data());
    for(auto P: myPlugins) P->writeItems(d);
    return d;
}

void PluginSaver::clearItems() {
    SegmentSaver::clearItems();
    for(auto P: myPlugins) P->clearItems();
}
