/// \file PluginSaver.cc
// -- Michael P. Mendenhall, 2019

#include "PluginSaver.hh"
#include "TermColor.hh"
#include "to_str.hh"

PluginSaver::PluginSaver(OutputManager* pnt, const Setting& S, const string& _path, const string& inflName):
SegmentSaver(pnt, _path, inflName) {
    if(S.getLength() && !fIn) {
        setMeta("settingname", S.getPath());
        setMeta("configstr", cfgString(lookupConfig(S)));
    }
    S.lookupValue("order", order);
}

void PluginSaver::buildPlugin(const string& pname, int& copynum, const Setting& cfg, bool skipUnknown) {
    auto o = skipUnknown? BaseFactory<SegmentSaver>::try_construct(pname, (SegmentSaver&)*this, cfg)
        : BaseFactory<SegmentSaver>::construct(pname, (SegmentSaver&)*this, cfg);

    if(!o) {
        printf("Skipping unknown plugin type '%s'!\n", pname.c_str());
        return;
    }

    string _rename = pname;
    if(copynum >= 0) _rename += "_"+to_str(copynum);
    string rn0 = _rename;
    cfg.lookupValue("rename",_rename);
    o->rename(_rename);
    byName[_rename] = o;
    myPlugins.push_back(o);
    if(rn0 == _rename) ++copynum;
}

void PluginSaver::init() {
    Config cfg;
    cfg.setAutoConvert(true);
    auto snm = getMeta("settingname");
    printf("Reconfiguring from saved setting '%s'\n", snm.c_str());
    cfg.readString(getMeta("configstr"));
    registerConfig(cfg);
    Configure(cfg.lookup(snm), true);
}

void PluginSaver::Configure(const Setting& S, bool skipUnknown) {
    if(myPlugins.size()) throw std::runtime_error("Multiple calls to PluginSaver::Configure");

    // configure plugins
    if(S.exists("plugins")) {
        auto& plugs = S["plugins"];
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
        auto Si = PS.getPlugin(P->path);
        if(!Si) continue;
        auto mm = P->compareKolmogorov(*Si);
        for(auto& kv: mm) m[P->path + "." + kv.first] = kv.second;
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
        auto Si = PS.getPlugin(P->path);
        if(Si) P->addSegment(*Si,sc);
        else printf("Warning: PluginSaver::addSegment missing matching plugin for '%s'\n", P->path.c_str());
    }
}

void PluginSaver::checkpoint(const SegmentSaver& Sprev) {
    auto& PS = dynamic_cast<const PluginSaver&>(Sprev);
    for(auto P: myPlugins) {
        auto Si = PS.getPlugin(P->path);
        if(Si) P->checkpoint(*Si);
        else printf("Warning: PluginSaver::checkpoint missing matching plugin for '%s'\n", P->path.c_str());
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
    ana_t0 = steady_clock::now();

    for(auto P: myPlugins) {
        auto t0 = steady_clock::now();
        P->startData();
        P->tProcess += std::chrono::duration<double>(steady_clock::now()-t0).count();
    }
}

void PluginSaver::processEvent() { for(auto P: myPlugins) P->processEvent(); }

void PluginSaver::finishData(bool f) {
    SegmentSaver::finishData(f);
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
            else vPi.push_back(PS->getPlugin(P->path));
        }
        P->defaultCanvas.cd();
        P->compare(vPi);
    }
}

void PluginSaver::BGSubtract(SegmentSaver& BG) {
    SegmentSaver::BGSubtract(BG);

    auto BGp = dynamic_cast<PluginSaver*>(&BG);
    if(!BGp) return;

    for(auto P: myPlugins) P->BGData = BGp->getPlugin(P->path);
}

void PluginSaver::calculateResults() {
    SegmentSaver::calculateResults();
    for(auto P: myPlugins) {
        auto t0 = steady_clock::now();
        printf("\n" TERMFG_BLUE "## " TERMFG_GREEN "%s CalculateResults" TERMFG_BLUE " ##" TERMSGR_RESET "\n\n", P->path.c_str());
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
        printf("* %s\n\t%.2f\t%.2f\t%.2f\t%.2f\t\t%.2f s\n", pb->path.c_str(),
                pb->tSetup, pb->tProcess, pb->tCalc, pb->tPlot, ttot);
        p_tSetup += pb->tSetup;
        p_tProcess += pb->tProcess;
        p_tCalc += pb->tCalc;
        p_tPlot += pb->tPlot;
        tall += ttot;
    }
    printf("----- Total ------\n\t%.2f\t%.2f\t%.2f\t%.2f\t\t%.2f s\n",
           p_tSetup, p_tProcess, p_tCalc, p_tPlot, tall);

    double tFramework = std::chrono::duration<double>(steady_clock::now() - ana_t0).count();
    printf("Framework time use: %.2f s\n\n", tFramework - tall);

    return tall;
}

TDirectory* PluginSaver::writeItems(TDirectory* d) {
    SegmentSaver::writeItems(d);
    printf("Writing plugins: ");
    for(auto& kv: byName) {
        printf(" %s", kv.first.c_str());
        kv.second->writeROOT();
    }
    printf("\n");
    return d;
}
