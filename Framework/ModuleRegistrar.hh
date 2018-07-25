/// \file ModuleRegistrar.hh Compile-time table of run-time loadable modules
// Michael P. Mendenhall, 2018

#ifndef MODULEREGISTRAR_HH
#define MODULEREGISTRAR_HH

#include <map>
using std::map;
#include <string>
using std::string;

/// Base class for registering a plugin to global table
template<class T>
class ModuleRegistrar {
public:
    /// construct instance by class name
    static T* construct(const string& n) {
        auto& m = M();
        auto it = m.find(n);
        return it == m.end()? nullptr : it->second.construct();
    }

protected:
    /// construct module
    virtual T* construct() const = 0;
    /// global map of available modules
    static map<string, ModuleRegistrar<T>&>& M() {
        static map<string, ModuleRegistrar<T>&> m;
        return m;
    };
};

/// Compile-time registration of dynamically-loadable plugins to base class
#define REGISTER_MODULE(NAME,BASE) \
class _##NAME##_Registrar: public ModuleRegistrar<BASE> { \
public: \
    _##NAME##_Registrar() { M().emplace(#NAME, *this); } \
    BASE* construct() const override { return new NAME; } \
}; \
static _##NAME##_Registrar the_##NAME##_Registrar;

#endif
