/// \file ObjectFactory.cc
// Michael P. Mendenhall, LLNL 2019

#include "ObjectFactory.hh"

template<class T>
T& singleton() {
    static T s;
    return s;
}

map<size_t, _ObjectFactory*>& _Factories::byIdx() { return singleton<map<size_t, _ObjectFactory*>>(); }
map<string, size_t>& _Factories::classIndices() { return singleton<map<string, size_t>>(); }

REGISTER_FACTORYOBJECT(_ObjectFactory,void)

