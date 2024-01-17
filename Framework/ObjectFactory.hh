/// @file ObjectFactory.hh ``Factory'' pattern for dynamic instantiation of objects by name
// -- Michael P. Mendenhall, LLNL 2019

#ifndef OBJECTFACTORY_HH
#define OBJECTFACTORY_HH

#include <map>
using std::map;
#include <string>
using std::string;
#include <typeinfo>
#include <sstream>
#include <vector>
using std::vector;
#include <algorithm>
#include <stdexcept>

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
    explicit _ObjectFactory(const string& cname): classname(cname) { }
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
    virtual base* construct(Args&&... a) const  = 0;
};

/// (static class) collection of factories, indexed by string name and unique identifier
class FactoriesIndex {
public:
    /// access map of factories by index = typehash<base, args> -> namehash : factory
    static map<size_t, map<size_t, _ObjectFactory&>>& index();

    /// index for particular construction
    template<typename... BArgs>
    static map<size_t, _ObjectFactory&>& indexFor() { return index()[typehash<BArgs...>()]; }

    /// sorted names for particular construction
    template<typename... BArgs>
    static vector<string> namesFor() {
        vector<string> v;
        for(auto& kv: indexFor<BArgs...>()) v.push_back(kv.second.classname);
        std::sort(v.begin(), v.end());
        return v;
    }

    /// hash string
    static size_t hash(const string& s) { return std::hash<string>{}(s); }

    /// show debugging list of registered classes
    static void display() {
        for(auto& kv: index()) {
            printf("--- %zu ---\n", kv.first);
            for(auto& kv2: kv.second) printf("%zu:\t'%s'\n", kv2.first, kv2.second.classname.c_str());
        }
    }
};

/// Concrete factory for a particular object type constructed with arguments
template<class B, class C, typename... Args>
class ObjectFactory: public _ArgsBaseFactory<B, Args...> {
public:
    /// Constructor, registering to list
    explicit ObjectFactory(const string& cname): _ArgsBaseFactory<B, Args...>(cname) {
        auto& idx = FactoriesIndex::indexFor<B, Args...>();
        auto h = FactoriesIndex::hash(cname);
        if(idx.count(h)) throw std::logic_error("Duplicate registration of class named '" + cname + "'");
        idx.emplace(h, *this);
    }
    /// Produce an object from arguments
    B* construct(Args&&... a) const override { return new C(std::forward<Args>(a)...); }
};

/// Error for failed class construction
class ConstructionError: public std::runtime_error {
public:
    /// Constructor
    explicit ConstructionError(const string& cname):
    std::runtime_error("Unknown class '" + cname + "' requested"),
    classname(cname) { }

    string classname;   ///< class name failing construction
};

/// Static factory for non-void object base types
template<class B>
class BaseFactory: protected FactoriesIndex {
public:
    /// Base type
    typedef B base;

    using FactoriesIndex::display;

    /// construct indexed class with arguments --- fails on unregistered index
    template<typename... Args>
    static base* construct(size_t i, Args&&... a) {
        return dynamic_cast<_ArgsBaseFactory<B, Args...>&>(indexFor<B, Args...>().at(i)).construct(std::forward<Args>(a)...);
    }

    /// show available options for construction
    template<typename... Args>
    static void displayConstructionOpts() {
        vector<string> vnames;
        for(auto& kv: indexFor<B, Args...>()) vnames.push_back(kv.second.classname);
        std::sort(vnames.begin(), vnames.end());
        for(const auto& n: vnames) printf("\t* %s\n", n.c_str());
    }

    /// Error for failed class construction of this type
    class BFConstructionError: public ConstructionError {
    public:
        /// Constructor
        using ConstructionError::ConstructionError;
    };

    /// construct named-class object with arguments; return nullptr if unavailable
    template<typename... Args>
    static base* try_construct(const string& classname, Args&&... a) {
        auto& mi = indexFor<B, Args...>();
        auto it = mi.find(hash(classname));
        return it == mi.end()?  nullptr : dynamic_cast<_ArgsBaseFactory<B, Args...>&>(it->second).construct(std::forward<Args>(a)...);
    }

    /// construct named-class object with arguments; throw with error message if unavailable
    template<typename... Args>
    static base* construct(const string& classname, Args&&... a) {
        auto o = try_construct(classname, std::forward<Args>(a)...);
        if(!o) {
            printf("Available options:\n");
            displayConstructionOpts<Args...>();
            throw BFConstructionError(classname);
        }
        return o;
    }
};

/// Compile-time registration of dynamically-constructable objects, default constructors
#define REGISTER_FACTORYOBJECT(NAME,BASE) static const ObjectFactory<BASE,NAME> the_##BASE##_##NAME##_Factory(#NAME);

#endif
