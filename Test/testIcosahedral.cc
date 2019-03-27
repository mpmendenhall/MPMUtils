/// \file testIcosahedral.cc Test of icosahdral point group code
// Michael P. Mendenhall, 2019

#include "Icosahedral.hh"
#include "CodeVersion.hh"
#include <stdlib.h>

int main(int, char**) {
    CodeVersion::display_code_version();

    using namespace Icosahedral;
    describe();

    return EXIT_SUCCESS;
}
