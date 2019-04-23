/// \file PluginSaver.cc
// -- Michael P. Mendenhall, 2019

#include "PluginSaver.hh"
#include "libconfig_readerr.hh"
#include "StringManip.hh"
#include <cassert>

PluginSaver::PluginSaver(OutputManager* pnt, const string& nm, const string& inflName):
SegmentSaver(pnt, nm, inflName) {
    configstr = registerAttrString("configstr", "");
}

void PluginSaver::LoadConfig(const string& fname) {
    Config cfg;
    readConfigFile(cfg, fname);
    Configure(cfg.getRoot(), false);
}

void PluginSaver::Reconfigure() {
    Config cfg;
    cfg.setAutoConvert(true);
    cfg.readString(configstr->String().Data());
    Configure(cfg.getRoot(), true);
}

void PluginSaver::buildPlugin(const string& pname, int& copynum, const Setting& cfg, bool skipUnknown) {
    auto i = pluginID(pname);
    if(!FactoriesIndex::has(i)) {
        if(skipUnknown) {
            printf("Skipping unknown plugin type '%s'!\n", pname.c_str());
            return;
        }
        fprintf(stderr,"Unknown plugin type '%s' configured! I die!\n", pname.c_str());
        throw std::runtime_error("Unknown plugin type");
    }

    auto o = BaseFactory<SegmentSaver>::construct(i, (SegmentSaver&)*this, cfg);
    string rename = pname;
    if(copynum >= 0) rename += "_"+to_str(copynum);
    string rn0 = rename;
    cfg.lookupValue("rename",rename);
    o->rename(rename);
    byName[rename] = o;
    myPlugins.push_back(o);
    if(rn0 == rename) ++copynum;
}

void PluginSaver::Configure(const Setting& cfg, bool skipUnknown) {
    // save copy of config to output
    auto srcfl = cfg.getSourceFile();
    if(srcfl) configstr->SetString(loadFileString(srcfl).c_str());

    // configure plugins
    if(cfg.exists("plugins")) {
        auto& plugs = cfg["plugins"];
        auto nplugs = plugs.getLength();
        for(int i=0; i<nplugs; i++) {
            string pname = plugs[i].getName();
            int copynum = -1;
            if(plugs[i].isList()) for(auto& c: plugs[i]) buildPlugin(pname, copynum, c, skipUnknown);
            else buildPlugin(pname, copynum, plugs[i], skipUnknown);
        }
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

SegmentSaver* PluginSaver::getPlugin(const string& nm) const {
    auto it = byName.find(nm);
    return it == byName.end()? nullptr : it->second;
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

void PluginSaver::processEvent() { for(auto P: myPlugins) P->processEvent(); }

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
            else vPi.push_back(PS->getPlugin(P->name));
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
    printf("Writing plugins: ");
    for(auto& kv: byName) {
        printf(" %s", kv.first.c_str());
        kv.second->writeItems(d);
    }
    printf("\n");
    return d;
}

void PluginSaver::clearItems() {
    SegmentSaver::clearItems();
    for(auto P: myPlugins) P->clearItems();
}
