/// @file ObjectFactory.hh ``Factory'' pattern for dynamic instantiation of objects by name
// -- Michael P. Mendenhall, LLNL 2019

#ifndef OBJECTFACTORY_HH
#define OBJECTFACTORY_HH

#include <map>
using std::map;
#include <string>
using std::string;
#include <typeinfo>
#include <typeindex>
using std::type_index;
#include <vector>
using std::vector;
#include <algorithm>
#include <stdexcept>

/*
 * _ObjectFactory: base polymorphic pointer for storing factories
 * _ArgsBaseFactory: Factory for a particular base class and constructor arguments
 * FactoriesIndex: static cass storing factories; able to construct object given <factory index>,args...
 * BaseFactory<B>: static class to access construction of base types B
 *
 * ObjectFactory: concrete instance of _ArgsBaseFactory for particular class and base class type
 * fancier uses may replace ObjFactory with custom types, performing additional setup in the Construct() function
*/

/// extract class name from type_index in printable (demangled) form
string demangled_classname(const type_index& i);

/// Inheritance base for factories
class _ObjectFactory {
public:
    /// make polymorphic
    virtual ~_ObjectFactory() { }
};

/// Access to collection-of-factories singleton
namespace FactoriesIndex {
    /// access map of factories by index<base, args> -> subclass name : ObjectFactory
    map<type_index, map<string, _ObjectFactory&>>& index();

    /// show debugging list of registered classes
    void display();
}

/// Factory base for particular arguments structure and base class
template<typename B, typename... Args>
class _ArgsBaseFactory: public _ObjectFactory {
public:
    /// Produce an object from arguments
    virtual B* construct(Args&&... a) const  = 0;
    /// lookup index
    static map<string, _ObjectFactory&>& index() { return FactoriesIndex::index()[type_index(typeid(_ArgsBaseFactory))]; }
    /// show available options for construction
    static void displayConstructionOpts() {
        vector<string> vnames;
        for(auto& kv: index()) vnames.push_back(kv.first);
        std::sort(vnames.begin(), vnames.end());
        for(const auto& n: vnames) printf("\t* %s\n", n.c_str());
    }

protected:
    /// register class in index
    void register_class(const string& cname) {
        auto& idx = index();
        if(idx.count(cname)) throw std::logic_error("Duplicate registration of class named '" + cname + "'");
        idx.emplace(cname, *this);
    }
};

/// Concrete factory for a particular object type constructed with arguments
template<class B, class C, typename... Args>
class ObjectFactory: protected _ArgsBaseFactory<B, Args...> {
public:
    /// Constructor, registering to list
    explicit ObjectFactory(const string& cname) { this->register_class(cname); }
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

/// Namespace class for constructing base type B
template<class B>
class BaseFactory {
public:
    /// construct named-class object with arguments; return nullptr if unavailable
    template<typename... Args>
    static B* try_construct(const string& classname, Args&&... a) {
        auto& mi = _ArgsBaseFactory<B, Args...>::index();
        auto it = mi.find(classname);
        return it == mi.end()?  nullptr : dynamic_cast<_ArgsBaseFactory<B, Args...>&>(it->second).construct(std::forward<Args>(a)...);
    }

    /// construct named-class object with arguments; throw with error message if unavailable
    template<typename... Args>
    static B* construct(const string& classname, Args&&... a) {
        auto o = try_construct(classname, std::forward<Args>(a)...);
        if(!o) {
            printf("Available options:\n");
            _ArgsBaseFactory<B, Args...>::displayConstructionOpts();
            throw ConstructionError(classname);
        }
        return o;
    }

protected:
    /// We never construct these
    BaseFactory() = delete;
};

/// Compile-time registration of dynamically-constructable objects, default constructors
#define REGISTER_FACTORYOBJECT(NAME,BASE) static const ObjectFactory<BASE,NAME> the_##BASE##_##NAME##_Factory(#NAME);

#endif
