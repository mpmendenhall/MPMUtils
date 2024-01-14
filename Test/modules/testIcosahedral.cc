/// @file testIcosahedral.cc Test of icosahdral point group code
// -- Michael P. Mendenhall, 2019

#include "Icosahedral.hh"
#include "ConfigFactory.hh"

REGISTER_EXECLET(testIcosahedral) {

    using namespace Icosahedral;
    describe();

    set<cayley_t::elem_t> S;
    S = {0,1};
    //for(auto& e: dodFaces[0].R) S.insert(e.i);
    quotient_t Q(CT, S);

    std::cout << "Subgroup is " << (isNormal(S,CT)? "" : "*not* ") << "normal in G.\n";

    std::cout << "\n" << Q.order() << " Equivalence classes:\n";
    size_t i = 0;
    for(auto c: Q.EQ) {
        std::cout << "\t" << c.first << "\t" << c.second.size() << "):";
        for(auto x: c.second) std::cout << " " << x;
        std::cout << "\n";
        i += c.second.size();
    }
    std::cout << "(total " << i << " elements).\n";
}
