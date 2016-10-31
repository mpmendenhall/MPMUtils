/// \file DynamicPluginSaver.cc
#include "DynamicPluginSaver.hh"

map<string, PluginRegistrar*> DynamicPluginSaver::builderTable;

void DynamicPluginSaver::Configure(Config& cfg) {
    if(cfg.exists("plugins")) {
        auto& pcfgs = cfg.lookup("plugins");
        for(auto pcfg = pcfgs.begin(); pcfg != pcfgs.end(); pcfg++) {
            string pname = pcfg->getName();
            auto it = builderTable.find(pname);
            if(it == builderTable.end()) {
                fprintf(stderr,"Unknown plugin type '%s' configured! I die!\n", pname.c_str());
                printf("Available plugins:\n");
                for(auto kv: builderTable) printf("\t%s\n", kv.first.c_str());
                printf("--------------------\n");
                throw;
            }
            myBuilders[pname] = it->second->makeBuilder(*pcfg);
        }
    }

    buildPlugins();
}

void DynamicPluginSaver::LoadConfig(const string& fname) {
    Config cfg;
    cfg.setAutoConvert(true);
    cfg.readFile(fname.c_str());
    Configure(cfg);
}
