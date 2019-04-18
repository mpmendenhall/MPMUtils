/// \file DynamicPluginSaver.cc
#include "DynamicPluginSaver.hh"
#include "libconfig_readerr.hh"
#include <TObjString.h>
#include "StringManip.hh"
#include <cassert>

DynamicPluginSaver::DynamicPluginSaver(OutputManager* pnt, const string& nm, const string& inflName):
PluginSaver(pnt, nm, inflName) {
    configstr = registerAttrString("configstr", "");
    assert(configstr);
}

void DynamicPluginSaver::Reconfigure() {
    if(configstr->String().Length()) {
        Config cfg;
        cfg.setAutoConvert(true);
        string s = configstr->String().Data();
        cfg.readString(s);
        Configure(cfg.getRoot(), true);
    } else printf("No configuration found in loaded file!\n");
}

void DynamicPluginSaver::addBuilder(const string& pname, int& copynum, const Setting& cfg, bool skipUnknown) {
    auto o = ObjectFactory<SegmentSaver>::construct(pname, cfg);
    if(!o) {
        if(skipUnknown) {
            printf("Skipping unknown plugin type '%s'!\n", pname.c_str());
            return;
        }
        fprintf(stderr,"Unknown plugin type '%s' configured! I die!\n", pname.c_str());
        throw std::runtime_error("Unknown plugin type");
    }

    throw nullptr; // TODO
    /*
    auto oo = dynamic_cast<PluginBuilder*>(o);
    if(!oo) {
        delete o;
        throw std::runtime_error("Incorrect object inheritance!");
    }

    string rename = pname;
    if(copynum >= 0) rename += "_"+to_str(copynum);
    string rn0 = rename;
    cfg.lookupValue("rename",rename);
    myBuilders.emplace(rename, oo);
    if(rn0 == rename) ++copynum;
    */
}

void DynamicPluginSaver::Configure(const Setting& cfg, bool skipUnknown) {
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
            if(plugs[i].isList()) for(auto& c: plugs[i]) addBuilder(pname, copynum, c, skipUnknown);
            else addBuilder(pname, copynum, plugs[i], skipUnknown);
        }
    }
    buildPlugins();
}

void DynamicPluginSaver::LoadConfig(const string& fname) {
    Config cfg;
    readConfigFile(cfg, fname);
}
