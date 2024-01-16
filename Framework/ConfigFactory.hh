/// @file ConfigFactory.hh Helper for ``factory'' construction from libconfig settings
// -- Michael P. Mendenhall, LLNL 2021

#ifndef CONFIGFACTORY_HH
#define CONFIGFACTORY_HH

#include "ObjectFactory.hh"
#include "ExplainConfig.hh"

/// Compile-time registration of BASE object constructed from const Setting&
#define REGISTER_CONFIG(NAME, BASE) static const ObjectFactory<BASE, NAME, const Setting&> the_##NAME##_CfgFactory(#NAME);

/// Construct configured object looked up from setting; return nullptr if unavailable
template<typename BASE, typename... Args>
BASE* try_constructCfgObj(const Setting& S, const string& dfltclass,  Args&&... a) {
    string defaultclass = dfltclass;
    ExplainConfig::lookupValue(S, "class", defaultclass, "class to construct");
    return BaseFactory<BASE>::try_construct(defaultclass, S, std::forward<Args>(a)...);
}

/// Construct configured object looked up from setting
template<typename BASE, typename... Args>
BASE* constructCfgObj(const Setting& S, const string& dfltclass, Args&&... a) {
    string defaultclass = dfltclass;
    ExplainConfig::lookupValue(S, "class", defaultclass, "class to construct", !dfltclass.size());
    return BaseFactory<BASE>::construct(defaultclass, S, std::forward<Args>(a)...);
}

/// Base class storing query info on configuration
class _Configurable {
public:
    /// Constructor
    explicit _Configurable(const Setting& S): Cfg(S) { Cfg.markused("class"); }
    /// Polymorphic Destructor
    virtual ~_Configurable() { }

protected:
    SettingsQuery Cfg;  ///< input configuration
};


/// Base class for a generic top-level configurable object
class Configurable: public _Configurable {
public:
    /// Constructor
    explicit Configurable(const Setting& S): _Configurable(S) { }
    /// Run configured operation
    virtual void run() { }
};

/// Pass-through Configurable generating "next"
class ConfigurableStage: public Configurable {
public:
    /// Constructor
    using Configurable::Configurable;
    /// construct "next"
    virtual void buildNext() { if(Cfg.show_exists("next", "next processing stage")) next = constructCfgObj<Configurable>(Cfg["next"], ""); }
    /// Destructor
    ~ConfigurableStage() { delete next; }
    /// Run configured operation
    void run() override { if(!next) buildNext(); if(next) next->run(); }

    Configurable* next = nullptr;   ///<   next run stage
};

/// register Configurable subclass
#define REGISTER_CONFIGURABLE(NAME) static ObjectFactory<Configurable, NAME, const Setting&> the_##NAME##_CfgblFactory(#NAME);

/// generate and register simple single-function Configurable --- follow by { brace enclosed } code block for run()
#define REGISTER_EXECLET(NAME) class NAME: public Configurable { \
    public: using Configurable::Configurable; void run() override; }; \
    REGISTER_CONFIGURABLE(NAME) void NAME::run()

#endif
