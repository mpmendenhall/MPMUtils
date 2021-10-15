/// \file mpmexamples.cc Run a configured example
// -- Michael P. Mendenhall 2020

#include "ConfigFactory.hh"
#include "GlobalArgs.hh"
#include "libconfig_readerr.hh"
#include "AnalysisStep.hh"
#include "AnaGlobals.hh"
#include "Exegete.hh"
#include "TermColor.hh"

#include <stdlib.h>
#include <stdio.h>

/// Execute configured analysis routine
int main(int argc, char** argv) {
    _EXPLAIN("Executing analysis code");

    printf(TERMSGR_ITALIC "\n");
    CodeVersion::display_code_version();
    printf(TERMSGR_RESET);

    if(argc < 2) {
        printf(TERMSGR_BOLD "\nArguments: mpmexamples <config file | class> [-argname argval(s) ...]\n\n" TERMSGR_RESET);
        printf("Available top-level classes:\n");
        BaseFactory<Configurable>::displayConstructionOpts<const Setting&>();
        return EXIT_FAILURE;
    }

    loadGlobalArgs(argc - 2, argv + 2);

    try {
        auto A = BaseFactory<Configurable>::try_construct(argv[1], NullSetting);
        AnalysisStep AS("mpmexamples");
        Config cfg;

        if(A) {
            printf(TERMSGR_BOLD TERMFG_YELLOW "\n-- Executing command-line-specified class '%s'\n" TERMSGR_RESET, argv[1]);
            AS.codename = argv[1];
        } else {
            printf(TERMFG_GREEN "\n-- Configuring from '%s'" TERMSGR_RESET "\n\n", argv[1]);
            readConfigFile(cfg, argv[1]);
            auto& S = registerConfig(cfg);

            A = constructCfgObj<Configurable>(S, "");
            S.lookupValue("class", AS.codename);
        }

        printf(TERMSGR_BOLD TERMFG_YELLOW "\n-- Begin analysis --" TERMSGR_RESET "\n\n");
        A->run();

        AS.tryAdd(A);
        AS.make_xmlout();

        delete A;
        printf(TERMSGR_BOLD TERMFG_GREEN "\n-- Analysis complete! --" TERMSGR_RESET "\n\n");

    } SHOW_CFG_ERRORS

    return EXIT_SUCCESS;
}
