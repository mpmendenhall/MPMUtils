/// \file Delta.cc main() for file comparator framework
// Michael P. Mendenhall, LLNL 2021

#include "CodeVersion.hh"
#include "GlobalArgs.hh"
#include "DeltaBase.hh"

#include <stdio.h>

/// Perform file comparison
int main(int argc, char** argv) {
    CodeVersion::display_code_version();

    if(argc < 3) {
        printf("Arguments: Delta <file 1> <file 2> [-out <dir>] [-as <type>]\n");
        return EXIT_FAILURE;
    }

    loadGlobalArgs(argc - 3, argv + 3);

    DeltaBase DB(argv[1], argv[2]);
    optionalGlobalArg("out", DB.outdir, "comparisons output directory");
    string astype = "automatic";
    optionalGlobalArg("as", astype, "comparison type");

    if(astype == "automatic") DB.inferType();
    else if(astype == "root") DB.comptype = DeltaBase::COMPARE_ROOT;
    else if(astype == "dir")  DB.comptype = DeltaBase::COMPARE_DIR;
    else if(astype == "diff") DB.comptype = DeltaBase::COMPARE_DIFF;
    else throw std::runtime_error("Unknown comparison type '" + astype + "'");

    return DB.compare()? EXIT_SUCCESS : EXIT_FAILURE;
}
