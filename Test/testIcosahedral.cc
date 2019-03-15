/// \file testIcosahedral.cc Test of icosahdral point group code

#include "Icosahedral.hh"
#include <stdlib.h>

int main(int argc, char** argv) {

    using namespace Icosahedral;
    describe();

    size_t e0 = nID;
    for(int i=1; i<argc; i++) {
        size_t e1 = atoi(argv[i]);
        std::cout << e1 << " * " << e0 << " = ";
        e0 = CT.apply(e1,e0);
        std::cout << e0 << "\n";
    }

    return EXIT_SUCCESS;
}
