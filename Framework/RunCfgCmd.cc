/// @file RunCfgCmd.cc

#include "RunCfgCmd.hh"
#include "ConfigFactory.hh"
#include "GlobalArgs.hh"
#include "AnalysisStep.hh"
#include "AnaGlobals.hh"
#include "TermColor.hh"

int RunCfgCmd::main(int argc, char** argv, const char* execname) {
    printf(TERMSGR_ITALIC "\n");
    CodeVersion::display_code_version();
    printf(TERMSGR_RESET);

    if(argc < 2) {
        printf(TERMSGR_BOLD "\nArguments: %s <config file | class> [-argname argval(s) ...]" TERMSGR_RESET "\n\n", execname);
        printf("Available top-level classes:\n");
        _ArgsBaseFactory<Configurable, const Setting&>::displayConstructionOpts();
        printf("\n");
        return EXIT_FAILURE;
    }

    loadGlobalArgs(argc - 2, argv + 2);
    pre_run();

    try {
        auto A = BaseFactory<Configurable>::try_construct(argv[1], NullSetting());
        AnalysisStep AS(execname);
        Config cfg;

        if(A) {
            printf(TERMSGR_BOLD TERMFG_YELLOW "\n-- Executing command-line-specified class '%s'" TERMSGR_RESET "\n", argv[1]);
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

        post_run();
        delete A;

        printf(TERMSGR_BOLD TERMFG_GREEN "\n-- Analysis complete! --" TERMSGR_RESET "\n\n");

    } SHOW_CFG_ERRORS
    catch(ConfigException& e) {
        printf(TERMFG_RED TERMSGR_BOLD "Exiting on configuration file error (%s)." TERMSGR_RESET "\n", e.what());
        return EXIT_FAILURE;
    } catch(std::runtime_error& e) {
        printf(TERMFG_RED TERMSGR_BOLD "Unable to proceed with execution as configured:\n\t%s" TERMSGR_RESET "\n", e.what());
        return EXIT_FAILURE;
    } catch(std::exception& e) {
        printf(TERMFG_RED TERMSGR_BOLD "Exiting due to error condition:\n\t%s" TERMSGR_RESET "\n", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
