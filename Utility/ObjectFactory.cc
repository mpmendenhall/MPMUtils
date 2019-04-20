/// \file ObjectFactory.cc
// Michael P. Mendenhall, LLNL 2019

#include "ObjectFactory.hh"

template<class T>
T& singleton() {
    static T s;
    return s;
}

map<size_t, _ObjectFactory*>& FactoriesIndex::index() { return singleton<map<size_t, _ObjectFactory*>>(); }
