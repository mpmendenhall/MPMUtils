/// \file ConfigFactory.hh Helper for ``factory'' construction from libconfig settings
// -- Michael P. Mendenhall, LLNL 2021

#ifndef CONFIGFACTORY_HH
#define CONFIGFACTORY_HH

#include "ObjectFactory.hh"
#include "libconfig_readerr.hh"
#include <stdexcept>

/// Compile-time registration of configuration-file-constructed classes
#define REGISTER_CONFIG(NAME, BASE) static ObjectFactory<BASE, NAME, const Setting&> the_##NAME##_CfgFactory(#NAME);

/// Construct class-named (possibly-null-configured) object
template<typename BASE, typename... Args>
BASE* constructCfgClass(const string& classname, bool failOK = false, const Setting& S = NullSetting,  Args&&... a) {
    return failOK? BaseFactory<BASE>::construct(classname, S, std::forward<Args>(a)...)
        : BaseFactory<BASE>::construct_or_throw(classname, S, std::forward<Args>(a)...);
}

/// Construct configured object
template<typename BASE, typename... Args>
BASE* constructCfgObj(const Setting& S, string defaultclass = "", bool failOK = false,  Args&&... a) {
    S.lookupValue("class", defaultclass);
    if(!defaultclass.size()) throw std::runtime_error("'class' not set in configuration");
    return constructCfgClass<BASE>(defaultclass, failOK, S, std::forward<Args>(a)...);
}

/// Base class for a generic top-level configurable object
class Configurable {
public:
    /// Constructor
    explicit Configurable(const Setting& S): Cfg(S) { }
    /// Polymorphic Destructor
    virtual ~Configurable() { }
    /// Run configured operation
    virtual void run() { }

protected:
    const Setting& Cfg; ///< input configuration
};

/// register Configurable subclass
#define REGISTER_CONFIGURABLE(NAME) static ObjectFactory<Configurable, NAME, const Setting&> the_##NAME##_CfgblFactory(#NAME);

/// generate and register simple single-function Configurable --- follow by { brace enclosed } code block for run()
#define REGISTER_EXECLET(NAME) class NAME: public Configurable { \
    public: using Configurable::Configurable; void run() override; }; \
    REGISTER_CONFIGURABLE(NAME) void NAME::run()

#endif
