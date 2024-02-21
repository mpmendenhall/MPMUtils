/// @file PluginSaver.cc
// -- Michael P. Mendenhall, 2019

#include "PluginSaver.hh"
#include "TermColor.hh"
#include "to_str.hh"
#include "GlobalArgs.hh"

PluginSaver::PluginSaver(OutputManager* pnt, const Setting& S, const string& _path, const string& inflName):
SegmentSaver(pnt, _path, inflName) {
    // if not reloading from file, save configuration to metadata ... will reload at "initialize()"
    if(S.getLength() && !fIn) {
        setMeta("settingname", S.getPath());
        setMeta("configstr", cfgString(lookupConfig(S)));
    }
}

void PluginSaver::buildPlugin(const string& pname, int& copynum, SettingsQuery& cfg, bool skipUnknown) {
    auto o = skipUnknown? BaseFactory<SegmentSaver>::try_construct(pname, (SegmentSaver&)*this, cfg)
        : BaseFactory<SegmentSaver>::construct(pname, (SegmentSaver&)*this, cfg);

    if(!o) {
        printf("Skipping unknown plugin type '%s'!\n", pname.c_str());
        return;
    }

    string _rename = pname;
    if(copynum >= 0) _rename += "_"+to_str(copynum);
    string rn0 = _rename;
    cfg.lookupValue("rename", _rename, "plugin renaming");
    o->rename(_rename);
    cfg.lookupValue("order", o->order, "plugin execution order");
    byName[_rename] = o;
    myPlugins.push_back(o);
    {
        auto t0 = steady_clock::now();
        o->initialize();
        o->tSetup += std::chrono::duration<double>(steady_clock::now()-t0).count();
    }
    if(rn0 == _rename) ++copynum;
}

void PluginSaver::initialize() {
    SegmentSaver::initialize();
    if(storedSQ) throw std::logic_error("Repeated initialization");

    // TODO better memory management
    auto cfg = new Config();
    cfg->setAutoConvert(true);
    auto snm = getMeta("settingname");
    printf("Reconfiguring from saved setting '%s'\n", snm.c_str());
    cfg->readString(getMeta("configstr"));
    registerConfig(*cfg);
    storedSQ = new SettingsQuery(cfg->lookup(snm));
    storedSQ->markused("class");
    Configure(*storedSQ, true);
}

void PluginSaver::Configure(SettingsQuery& S, bool skipUnknown) {
    if(myPlugins.size()) throw std::runtime_error("Multiple calls to PluginSaver::Configure");

    // configure plugins
    auto& ps = S.get("plugins", "analysis plugins");
    for(auto& p: ps) {
        string pname = p.getName();
        ps.markunused(pname);
        ps.show_exists(pname, "plugin settings");
        int copynum = -1;
        if(p->isList()) {
            for(auto& c: p) buildPlugin(pname, copynum, c, skipUnknown);
        } else buildPlugin(pname, copynum, p, skipUnknown);
    }

    std::sort(myPlugins.begin(), myPlugins.end(),
              [](const SegmentSaver* a, const SegmentSaver* b) { return a->order < b->order; });

    printf("\n");
    if(optionalGlobalArg("plotformat", printsfx, "plot output format")) setPrintSuffix(printsfx);
    printf("\n");
}

map<string,float> PluginSaver::compareKolmogorov(const SegmentSaver& S) const {
    auto m = SegmentSaver::compareKolmogorov(S);
    auto& PS = dynamic_cast<const PluginSaver&>(S);
    for(const auto P: myPlugins) {
        const auto* Si = PS.getPlugin(P->path);
        if(!Si) continue;
        auto mm = P->compareKolmogorov(*Si);
        for(const auto& kv: mm) m[P->path + "." + kv.first] = kv.second;
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

void PluginSaver::normalize_runtime() {
    SegmentSaver::normalize_runtime();
    for(auto P: myPlugins) P->normalize_runtime();
}

void PluginSaver::normalize() {
    SegmentSaver::normalize();
    for(auto P: myPlugins) P->normalize();
}

void PluginSaver::addSegment(const SegmentSaver& S, double sc) {
    SegmentSaver::addSegment(S);
    auto& PS = dynamic_cast<const PluginSaver&>(S);
    for(auto P: myPlugins) {
        const auto* Si = PS.getPlugin(P->path);
        if(Si) P->addSegment(*Si,sc);
        else printf("Warning: PluginSaver::addSegment missing matching plugin for '%s'\n", P->path.c_str());
    }
}

void PluginSaver::checkpoint(const SegmentSaver& Sprev) {
    auto& PS = dynamic_cast<const PluginSaver&>(Sprev);
    for(auto P: myPlugins) {
        const auto* Si = PS.getPlugin(P->path);
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

void PluginSaver::signal(datastream_signal_t s) {


    if(s == DATASTREAM_INIT) {
        SegmentSaver::signal(s);
        ana_t0 = steady_clock::now();
        for(auto P: myPlugins) {
            auto t0 = steady_clock::now();
            P->signal(s);
            P->tProcess += std::chrono::duration<double>(steady_clock::now()-t0).count();
        }
    } else if(s == DATASTREAM_END) {
        for(auto P: myPlugins) {
            auto t0 = steady_clock::now();
            P->signal(s);
            P->tProcess += std::chrono::duration<double>(steady_clock::now()-t0).count();
        }
        SegmentSaver::signal(s);
    } else {
        for(auto P: myPlugins) P->signal(s);
        SegmentSaver::signal(s);
    }
}

void PluginSaver::compare(const vector<SegmentSaver*>& v) {
    SegmentSaver::compare(v);

    vector<PluginSaver*> vP;
    for(auto SS: v) vP.push_back(dynamic_cast<PluginSaver*>(SS));

    for(auto P: myPlugins) {
        vector<SegmentSaver*> vPi;
        for(const auto PS: vP) {
            if(!PS) vPi.push_back(nullptr);
            else vPi.push_back(PS->getPlugin(P->path));
        }
        P->defaultCanvas.cd();
        P->compare(vPi);
    }
}

void PluginSaver::BGSubtract(SegmentSaver& BG) {
    SegmentSaver::BGSubtract(BG);

    const auto BGp = dynamic_cast<const PluginSaver*>(&BG);
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
