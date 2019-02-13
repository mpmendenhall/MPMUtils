/// \file ObjectFactory.hh ``Factory'' pattern for dynamic instantiation of objects
// Michael P. Mendenhall, LLNL 2019

#ifndef OBJECTFACTORY_HH
#define OBJECTFACTORY_HH

#include <map>
using std::map;
#include <string>
using std::string;
#include <typeinfo>

/// Base from which all constructable objects inherit
class FactoryObject { };

/// Base from which object factories are defined
class ObjectFactory {
public:
    /// index for classes by name
    static map<string, size_t>& classIndices();
    /// names for classes by index
    static map<size_t, string>& classNames();
    /// access global map of factories by type index
    static map<size_t, ObjectFactory*>& byIdx();

    /// construct by class name (returns nullptr if not registered)
    static FactoryObject* construct(const string& classname);
    /// construct by class typeid
    static FactoryObject* construct(size_t t);

protected:
    /// construct correct subclass of FactoryObject
    virtual FactoryObject* construct() const = 0;
};

/// Base for registering a class by name
template<typename T>
class _NameRegistrar {
public:
    _NameRegistrar(const string& tname) {
        ObjectFactory::classIndices().emplace(tname, typeid(T).hash_code());
        ObjectFactory::classNames().emplace(typeid(T).hash_code(), tname);
    }
};

/// Factory for a particular object type
template<class C>
class _ObjectFactory: public ObjectFactory, protected _NameRegistrar<C> {
public:
    /// Constructor
    _ObjectFactory(const string& cname): _NameRegistrar<C>(cname) { ObjectFactory::byIdx().emplace(typeid(C).hash_code(), this); }
    /// Produce an object
    FactoryObject* construct() const override { return new C; }
};

/// Compile-time registration of class name
#define REGISTER_TYPENAME(NAME) static _NameRegistrar<NAME> the_##NAME##_NameRegistrar(#NAME);
/// Compile-time registration of dynamically-constructable objects
#define REGISTER_FACTORYOBJECT(NAME) static _ObjectFactory<NAME> the_##NAME##_Factory(#NAME);

#endif
