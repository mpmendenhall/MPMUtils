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

template<typename... Args>
struct _args_t { };

/// convenience function for type hash
template<typename... T>
constexpr size_t typehash() { return typeid(_args_t<T...>).hash_code(); }

/// Inheritance base for factories; Singleton derived-class instances provide class metadata.
class _ObjectFactory {
public:
    /// make polymorphic
    virtual ~_ObjectFactory() { }
    /// name of class to be constructed
    string classname;
};

/// Factory base for particular arguments structure
template<typename... Args>
class _ArgsFactory: public _ObjectFactory {
public:
    /// Produce an object (of unspecified type) from arguments
    virtual void* vconstruct(Args&&... a) const = 0;
};

/// Factory base for particular arguments structure and base class
template<typename B, typename... Args>
class _ArgsBaseFactory: public _ArgsFactory<Args...> {
public:
    /// base class
    typedef B base;
    /// Produce an object from arguments
    virtual B* bconstruct(Args&&... a) const  = 0;
    /// Construct an object with forwarded args
    void* vconstruct(Args&&... a) const override { return (void*)bconstruct(std::forward<Args>(a)...); }
};

/// (static class) collection of factories by object type
class _Factories {
public:
    /// get factory for named class / args
    template<typename... Args>
    static _ArgsFactory<Args...>* getFactory(const string& classname) {
        auto it = classIndices().find(argsname<Args...>(classname));
        return it == classIndices().end()? nullptr : dynamic_cast<_ArgsFactory<Args...>*>(byIdx().at(it->second));
    }

    /// construct object with arguments --- fails on unregistered index
    template<typename... Args>
    static void* construct(size_t i, Args&&... a) {
        auto& f = dynamic_cast<_ArgsFactory<Args...>&>(*byIdx().at(i));
        return f.vconstruct(std::forward<Args>(a)...);
    }

    /// construct by name with arguments (returns nullptr if not registered)
    template<typename... Args>
    static void* construct(const string& classname, Args&&... a) {
        auto f = getFactory(classname);
        return f? f->vconstruct(std::forward<Args>(a)...) : nullptr;
    }

    /// look up class name for index
    static const string& nameOf(size_t i) { return byIdx().at(i)->classname; }

    /// object string representation helper
    template<typename T>
    static string to_str(T x) { std::stringstream ss; ss << x; return ss.str(); }
    /// name for factory for class with constructor args
    template<typename... Args>
    static string argsname(const string& n) { return n + "_" + to_str(typehash<Args...>()); }

    /// access map "name_<arg hash>" to unique index
    static map<string, size_t>& classIndices();
    /// access map of factories by index
    static map<size_t, _ObjectFactory*>& byIdx();
};

/// Factory for a particular object type constructed with arguments
template<class B, class C, typename... Args>
class _KnownObjFactory: public _ArgsBaseFactory<B, Args...> {
public:
    /// unique identifier for class + args
    static size_t idx() { return typehash<_KnownObjFactory>(); }

    /// Constructor, registering to list
    _KnownObjFactory(const string& cname) {
        this->classname = cname;
        auto n = _Factories::argsname<Args...>(cname); // name with arguments
        _Factories::classIndices().emplace(n, idx());
        _Factories::byIdx().emplace(this->idx(), this);
    }
};


/// Factory for a particular object type constructed with arguments
template<class B, class C, typename... Args>
class KnownObjFactory: public _KnownObjFactory<B,C,Args...> {
public:
    /// inherit constructors
    using _KnownObjFactory<B,C,Args...>::_KnownObjFactory;
    /// Produce an object from arguments
    B* bconstruct(Args&&... a) const override { return new C(std::forward<Args>(a)...); }
};

/// Static factory for non-void object base types
template<class B>
class ObjectFactory: protected _Factories {
public:
    /// Base type
    typedef B base;

    /// construct named-class object with arguments
    template<typename... Args>
    static base* construct(const string& classname, Args&&... a) {
        auto f = dynamic_cast<_ArgsBaseFactory<B, Args...>*>(getFactory<Args...>(classname));
        return f? f->bconstruct(std::forward<Args>(a)...) : nullptr;
    }
};

/// Compile-time registration of dynamically-constructable objects, default constructors
#define REGISTER_FACTORYOBJECT(NAME,BASE) static KnownObjFactory<BASE,NAME> the_##BASE##_##NAME##_Factory(#NAME);

#endif
