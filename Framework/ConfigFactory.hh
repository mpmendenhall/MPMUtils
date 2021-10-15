/// \file ConfigFactory.hh Helper for ``factory'' construction from libconfig settings
// -- Michael P. Mendenhall, LLNL 2021

#ifndef CONFIGFACTORY_HH
#define CONFIGFACTORY_HH

#include "ObjectFactory.hh"
#include "libconfig_readerr.hh"

/// Compile-time registration of BASE object constructed from const Setting&
#define REGISTER_CONFIG(NAME, BASE) static ObjectFactory<BASE, NAME, const Setting&> the_##NAME##_CfgFactory(#NAME);

/// Construct configured object looked up from setting; return nullptr if unavailable
template<typename BASE, typename... Args>
BASE* try_constructCfgObj(const Setting& S, const string& dfltclass,  Args&&... a) {
    string defaultclass = dfltclass;
    S.lookupValue("class", defaultclass);
    return BaseFactory<BASE>::try_construct(defaultclass, S, std::forward<Args>(a)...);
}

/// Construct configured object looked up from setting
template<typename BASE, typename... Args>
BASE* constructCfgObj(const Setting& S, const string& dfltclass, Args&&... a) {
    string defaultclass = dfltclass;
    S.lookupValue("class", defaultclass);
    if(!defaultclass.size()) throw std::runtime_error("'class' not set in configuration");
    return BaseFactory<BASE>::construct(defaultclass, S, std::forward<Args>(a)...);
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
