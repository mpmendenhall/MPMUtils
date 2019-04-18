/// \file ObjectFactory.cc
// Michael P. Mendenhall, LLNL 2019

#include "ObjectFactory.hh"

template<class T>
T& singleton() {
    static T s;
    return s;
}

map<size_t, ObjectFactory*>& ObjectFactory::byIdx() { return singleton<map<size_t, ObjectFactory*>>(); }
map<string, size_t>& ObjectFactory::classIndices() { return singleton<map<string, size_t>>(); }

REGISTER_FACTORYOBJECT(FactoryObject)
