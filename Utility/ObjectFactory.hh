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

/// Base from which all constructable objects inherit
class FactoryObject {
public:
    /// Destructor to make class polymorphic
    virtual ~FactoryObject() { }
};

/// convenience function for type hash
template<typename T = void>
constexpr size_t typehash() { return typeid(T).hash_code(); }

template<typename... Args>
class ObjArgsFactory;

/// Base from which object factories are defined.
/// Singleton derived-class instances provide class metadata.
class ObjectFactory {
public:
    /// Make class polymorphic
    virtual ~ObjectFactory() {}

    /// construct object with arguments --- fails on unregistered index
    template<typename... Args>
    static FactoryObject* construct(size_t i, Args&&... a) {
        auto& f = dynamic_cast<ObjArgsFactory<Args...>&>(*byIdx().at(i));
        return f._construct(std::forward<Args>(a)...);
    }

    /// construct by name with arguments (returns nullptr if not registered)
    template<typename... Args>
    static FactoryObject* construct(const string& classname, Args&&... a) {
        auto it = classIndices().find(argsname<Args...>(classname));
        return it == classIndices().end()? nullptr : construct(it->second, std::forward<Args>(a)...);
    }

    /// look up class name for index
    static const string& nameOf(size_t i) { return byIdx().at(i)->classname; }

    /// object string representation helper
    template<typename T>
    static string to_str(T x) { std::stringstream ss; ss << x; return ss.str(); }
    /// name for factory for class with constructor args
    template<typename... Args>
    static string argsname(const string& n) { return n + "_" + to_str(typehash<Args...>()); }

    /// class name
    const string classname;

protected:
    /// access map "name_<arg hash>" to unique index
    static map<string, size_t>& classIndices();
    /// access map of factories by index
    static map<size_t, ObjectFactory*>& byIdx();

    /// Constructor with class name
    ObjectFactory(const string& cname): classname(cname) { }
};

/// Factory base for particular arguments structure
template<typename... Args>
class ObjArgsFactory: public ObjectFactory {
public:
    /// Constructor with class name
    using ObjectFactory::ObjectFactory;
    /// Produce an object from arguments
    virtual FactoryObject* _construct(Args&&... a) const = 0;
};

/// Factory for a particular object type constructed with arguments
template<class C, typename... Args>
class _ObjArgsFactory: public ObjArgsFactory<Args...> {
public:
    /// unique identifier for class + args
    static size_t idx() { return typehash<_ObjArgsFactory>(); }

    /// Constructor
    _ObjArgsFactory(const string& cname): ObjArgsFactory<Args...>(cname) {
        auto n = ObjectFactory::argsname<Args...>(cname); // name with arguments
        ObjectFactory::classIndices().emplace(n, idx());
        ObjectFactory::byIdx().emplace(this->idx(), this);
    }

    /// Construct an object with forwarded args
    FactoryObject* _construct(Args&&... a) const override { return new C(std::forward<Args>(a)...); }
};

/// Compile-time registration of dynamically-constructable objects, default constructors
#define REGISTER_FACTORYOBJECT(NAME) static _ObjArgsFactory<NAME> the_##NAME##_Factory(#NAME);


#endif
