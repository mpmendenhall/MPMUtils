/// \file ObjectFactory.cc
// Michael P. Mendenhall, LLNL 2019

#include "ObjectFactory.hh"

FactoryObject* ObjectFactory::construct(const string& classname) {
    auto& ns = classIndices();
    auto it = ns.find(classname);
    return it == ns.end()? nullptr : construct(it->second);
}

FactoryObject* ObjectFactory::construct(size_t t) {
    auto& m = byIdx();
    auto it = m.find(t);
    return it == m.end()? nullptr : it->second->construct();
}

template<class T>
T& singleton() {
    static T s;
    return s;
}

map<size_t, ObjectFactory*>& ObjectFactory::byIdx() { return singleton<map<size_t, ObjectFactory*>>(); }
map<string, size_t>& ObjectFactory::classIndices() { return singleton<map<string, size_t>>(); }
map<size_t, string>& ObjectFactory::classNames() { return singleton<map<size_t, string>>(); }

REGISTER_TYPENAME(ObjectFactory)
REGISTER_FACTORYOBJECT(FactoryObject)
