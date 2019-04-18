/// \file DynamicPluginSaver.hh System for dynamically loading analyzer plugins by config file
// Michael P. Mendenhall, LLNL 2019

#ifndef DYNAMICPLUGINSAVER_HH
#define DYNAMICPLUGINSAVER_HH

#include "PluginSaver.hh"
#include "ObjectFactory.hh"
#include "libconfig.h++"
using namespace libconfig;

class DynamicPluginSaver: public PluginSaver {
public:
    /// Constructor
    DynamicPluginSaver(OutputManager* pnt, const string& nm = "DynamicPluginSaver", const string& inflName = "");
    /// Configure from libconfig object, dynamically loading plugins
    virtual void Configure(const Setting& cfg, bool skipUnknown = false);
    /// Configure, loading config by filename
    void LoadConfig(const string& fname);
    /// Configure, loading from input file
    void Reconfigure();

protected:
    TObjString* configstr;  ///< configuration file string

    /// add named builder
    void addBuilder(const string& pname, int& copynum, const Setting& cfg, bool skipUnknown);
};

// Dynamic plugin builder requirements: return SegmentSaver*; constructor args (SegmentSaver&, const Setting&)

/// Base class for constructing configuration-based plugins, with parent-class recast
template <class Plug, class Base>
class ConfigPluginBuilder: public _KnownObjFactory<SegmentSaver, Plug, SegmentSaver&, const Setting&> {
public:
    using _KnownObjFactory<SegmentSaver, Plug, SegmentSaver&, const Setting&>::_KnownObjFactory;

    /// Re-casting plugin construction
    SegmentSaver* bconstruct(SegmentSaver& pnt, const Setting& S) const override {
        auto t0 = steady_clock::now();
        auto P = new Plug(dynamic_cast<Base&>(pnt), S);
        P->tSetup += std::chrono::duration<double>(steady_clock::now()-t0).count();
        S.lookupValue("order", P->order);
        return P;
    }
};

/// Compile-time registration of dynamically-loadable plugins
#define REGISTER_PLUGIN(NAME,BASE) static ConfigPluginBuilder<NAME,BASE> the_##BASE##_##NAME##_PluginBuilder(#NAME);

#endif
