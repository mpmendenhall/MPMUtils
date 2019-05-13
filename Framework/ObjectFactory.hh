/// \file ObjectFactory.hh ``Factory'' pattern for dynamic instantiation of objects by name
// Michael P. Mendenhall, LLNL 2019

#ifndef OBJECTFACTORY_HH
#define OBJECTFACTORY_HH

#include <map>
using std::map;
#include <string>
using std::string;
#include <typeinfo>
#include <sstream>

/*
 * _ObjectFactory: base polymorphic pointer for storing factories
 * _ArgsFactory: base for any factory with particular arguments structure
 * _ArgsBaseFactory: _ArgsFactory for a particular base class
 * FactoriesIndex: static cass storing factories; able to construct object given <factory index>,args...
 * BaseFactory<B>: static class to access construction of base types B
 *
 * ObjectFactory: concrete instance of _ArgsBaseFactory for particular class and base class type
 * fancier uses may replace _KnownObjFactory with custom types, performing additional setup
*/

/// wrapper for variable arguments list
template<typename... Args>
struct _args_t { };
/// convenience function for type hash
template<typename... T>
constexpr size_t typehash() { return typeid(_args_t<T...>).hash_code(); }

/// Inheritance base for factories; Singleton derived-class instances provide class metadata.
class _ObjectFactory {
public:
    /// constructor with class name
    _ObjectFactory(const string& cname): classname(cname) { }
    /// make polymorphic
    virtual ~_ObjectFactory() { }
    /// name of class to be constructed
    string classname;
};

/// Factory base for particular arguments structure
template<typename... Args>
class _ArgsFactory: public _ObjectFactory {
public:
    /// inherit constructor
    using _ObjectFactory::_ObjectFactory;
    /// Produce an object (of unspecified type) from arguments
    virtual void* vconstruct(Args&&... a) const = 0;
};

/// Factory base for particular arguments structure and base class
template<typename B, typename... Args>
class _ArgsBaseFactory: public _ArgsFactory<Args...> {
public:
    /// base class
    typedef B base;
    /// inherit constructor
    using _ArgsFactory<Args...>::_ArgsFactory;
    /// Produce an object from arguments
    virtual B* bconstruct(Args&&... a) const  = 0;
    /// Construct an object with forwarded args
    void* vconstruct(Args&&... a) const override { return (void*)bconstruct(std::forward<Args>(a)...); }
};

/// (static class) collection of factories, indexed by string name and unique identifier
class FactoriesIndex {
public:
    /// construct object with arguments --- fails on unregistered index
    template<typename... Args>
    static void* construct(size_t i, Args&&... a) {
        auto& f = dynamic_cast<_ArgsFactory<Args...>&>(*index().at(i));
        return f.vconstruct(std::forward<Args>(a)...);
    }

    /// check if index available
    static bool has(size_t i) { return index().count(i); }

    /// access map of factories by index = typehash<factory type>
    static map<size_t, _ObjectFactory*>& index();

    /// show debugging list of registered classes
    static void display() {
        for(auto& kv: index()) printf("%zu:\t%s\n", kv.first, kv.second->classname.c_str());
    }
};

/// identifier index by class name (+ custom-class factory specialization name)
template<typename... BArgs>
size_t factoryID(const string& cname, const string& fname = "_default") { return std::hash<string>{}(cname+fname) ^ typehash<BArgs...>(); }

/// Concrete factory for a particular object type constructed with arguments
template<class B, class C, typename... Args>
class ObjectFactory: public _ArgsBaseFactory<B, Args...> {
public:
    /// Constructor, registering to list
    ObjectFactory(const string& cname): _ArgsBaseFactory<B, Args...>(cname) {
        auto i = factoryID<B, Args...>(cname);
        FactoriesIndex::index().emplace(i, this);
    }
    /// Produce an object from arguments
    B* bconstruct(Args&&... a) const override { return new C(std::forward<Args>(a)...); }
};

/// Static factory for non-void object base types
template<class B>
class BaseFactory: protected FactoriesIndex {
public:
    /// Base type
    typedef B base;

    /// construct indexed class with arguments --- fails on unregistered index
    template<typename... Args>
    static base* construct(size_t i, Args&&... a) {
        auto& f = dynamic_cast<_ArgsBaseFactory<B, Args...>&>(*index().at(i));
        return f.bconstruct(std::forward<Args>(a)...);
    }

    /// get index for named class
    template<typename... Args>
    static size_t classID(const string& cname) { return factoryID<B, Args...>(cname); }

    /// construct named-class object with arguments; nullptr if unavailable
    template<typename... Args>
    static base* construct(const string& classname, Args&&... a) {
        auto it = index().find(classID<Args...>(classname));
        return it == index().end()? nullptr : dynamic_cast<_ArgsBaseFactory<B, Args...>&>(*it->second).bconstruct(std::forward<Args>(a)...);
    }
};

/// Compile-time registration of dynamically-constructable objects, default constructors
#define REGISTER_FACTORYOBJECT(NAME,BASE) static ObjectFactory<BASE,NAME> the_##BASE##_##NAME##_Factory(#NAME);

#endif
