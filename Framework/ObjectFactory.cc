/// @file ObjectFactory.cc
// -- Michael P. Mendenhall, LLNL 2019

#include "ObjectFactory.hh"

template<class T>
T& singleton() {
    static T s;
    return s;
}

map<type_index, map<string, _ObjectFactory&>>& FactoriesIndex::index() { return singleton<map<type_index, map<string, _ObjectFactory&>>>(); }

#include <cxxabi.h>
#include <cstdlib>

string demangled_classname(const type_index& i) {
    int status = 0;
    auto realname = abi::__cxa_demangle(i.name(), NULL, NULL, &status);
    string s(realname);
    std::free(realname);
    return s;
}

void FactoriesIndex::display() {
    for(auto& kv: index()) {
        printf("--- %s ---\n", demangled_classname(kv.first).c_str());
        for(auto& kv2: kv.second) printf("\t%s\n", kv2.first.c_str());
    }
}
