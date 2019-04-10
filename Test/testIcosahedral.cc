/// \file testIcosahedral.cc Test of icosahdral point group code
// Michael P. Mendenhall, 2019

#include "Icosahedral.hh"
#include "CodeVersion.hh"
#include <stdlib.h>

int main(int, char**) {
    CodeVersion::display_code_version();

    using namespace Icosahedral;
    describe();

    //return EXIT_SUCCESS;

    set<cayley_t::elem_t> S;
    //S = {0,1};
    for(auto& e: dodFaces[0].R) S.insert(e.i);
    quotient_t Q(CT, S);

    std::cout << "\n" << Q.order() << " Equivalence classes:\n";
    size_t i = 0;
    for(auto c: Q.EQ) {
        std::cout << "\t" << c.first << "\t" << c.second.size() << "):";
        for(auto x: c.second) std::cout << " " << x;
        std::cout << "\n";
        i += c.second.size();
    }
    std::cout << "(total " << i << " elements).\n";

    return EXIT_SUCCESS;
}
