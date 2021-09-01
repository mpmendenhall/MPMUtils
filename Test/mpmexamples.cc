/// \file mpmexamples.cc Run a configured example
// -- Michael P. Mendenhall 2020

#include "ConfigFactory.hh"
#include "GlobalArgs.hh"
#include "libconfig_readerr.hh"
#include "AnalysisStep.hh"
#include "Exegete.hh"

#include <stdlib.h>
#include <stdio.h>

/// Execute configured analysis routine
int main(int argc, char** argv) {
    _EXPLAIN("Executing mpmexamples");
    displayCodeVersion();

    if(argc < 2) {
        printf("Arguments: ./mpmexamples <config file | class> [-argname argval(s) ...]\n");
        FactoriesIndex::display();
        return EXIT_FAILURE;
    }

    loadGlobalArgs(argc - 2, argv + 2);

    auto A = constructCfgClass<Configurable>(argv[1], true);
    AnalysisStep AS("mpmexamples");
    Config cfg;

    if(A) {
        printf("Executing command-line-specified class '%s'\n", argv[1]);
        AS.codename = argv[1];
    } else {
        _EXPLAINVAL("Loading configuration file", argv[1]);
        readConfigFile(cfg, argv[1]);
        auto& S = registerConfig(cfg);

        try {
            A = constructCfgObj<Configurable>(S);
            S.lookupValue("class", AS.codename);
            _EXPLAINVAL("Executing class", AS.codename);
        } catch(SettingException& e) {
            printf("Configuration SettingException (wrong type) at '%s'\n", e.getPath());
            throw;
        } catch(ConfigException& e) {
            printf("Exiting on configuration error.\n");
            throw;
        }
    }

    A->run();

    _EXPLAIN("Generating .xml metadata file");
    AS.tryAdd(A);
    AS.make_xmlout();

    _EXPLAIN("Cleanup");
    delete A;
    printf("Analysis complete!\n");

    return EXIT_SUCCESS;
}
