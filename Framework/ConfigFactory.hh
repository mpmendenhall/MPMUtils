/// \file ConfigFactory.hh Helper for ``factory'' construction from libconfig settings
// -- Michael P. Mendenhall, LLNL 2019

#ifndef CONFIGFACTORY_HH
#define CONFIGFACTORY_HH

#include "ObjectFactory.hh"
#include "libconfig_readerr.hh"
#include <stdexcept>

/// Compile-time registration of configuration-file-constructed classes
#define REGISTER_CONFIG(NAME, BASE) static ObjectFactory<BASE, NAME, const Setting&> the_##NAME##_CfgFactory(#NAME);

/// Compile-time registration of configuration-file-constructed classes with const parent ref
#define REGISTER_CPCONFIG(NAME, BASE, PARENT) static ObjectFactory<BASE, NAME, const PARENT&, const Setting&> the_##NAME##_CPFactory(#NAME);

/// Compile-time registration of configuration-file-constructed classes with mutable parent ref
#define REGISTER_MPCONFIG(NAME, BASE, PARENT) static ObjectFactory<BASE, NAME, PARENT&, const Setting&> the_##NAME##_MPFactory(#NAME);

/// Construct class-named (possibly-null-configured) object
template<typename BASE>
BASE* constructCfgClass(const string& classname, bool failOK = false, const Setting& S = NullSetting) {
    auto o = BaseFactory<BASE>::construct(classname, S);
    if(!o && !failOK) {
        FactoriesIndex::display();
        throw std::runtime_error("Unknown class '" + classname + "' requested");
    }
    return o;
}

/// Construct configured object
template<typename BASE>
BASE* constructCfgObj(const Setting& S, string defaultclass = "", bool failOK = false) {
    S.lookupValue("class", defaultclass);
    if(!defaultclass.size()) throw std::runtime_error("'class' not set in configuration");
    return constructCfgClass<BASE>(defaultclass, failOK, S);
}

/// Construct configured object
template<typename BASE, typename PARENT>
BASE* constructMPCfgObj(PARENT& P, const Setting& S, string defaultclass = "", bool failOK = false) {
    S.lookupValue("class", defaultclass);
    if(!defaultclass.size()) throw std::runtime_error("'class' not set in configuration");
    auto o = BaseFactory<BASE>::construct(defaultclass, P, S);
    if(!o && !failOK) throw std::runtime_error("Unknown class '"+defaultclass+"' requested in configuration");
    return o;
}

/// Construct configured object
template<typename BASE, typename PARENT>
BASE* constructCPCfgObj(const PARENT& P, const Setting& S, string defaultclass = "", bool failOK = false) {
    S.lookupValue("class", defaultclass);
    if(!defaultclass.size()) throw std::runtime_error("'class' not set in configuration");
    auto o = BaseFactory<BASE>::construct(defaultclass, P, S);
    if(!o && !failOK) throw std::runtime_error("Unknown class '"+defaultclass+"' requested in configuration");
    return o;
}

/// Base class for a generic top-level configurable object
class Configurable {
public:
    /// Constructor
    explicit Configurable(const Setting&) { }
    /// Destructor
    virtual ~Configurable() { }
    /// Run configured operation
    virtual void run() { }
};

#define REGISTER_CONFIGURABLE(NAME) static ObjectFactory<Configurable, NAME, const Setting&> the_##NAME##_CfgblFactory(#NAME);

#endif
