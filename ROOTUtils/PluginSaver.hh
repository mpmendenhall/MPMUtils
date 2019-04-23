/// \file PluginSaver.hh
// -- Michael P. Mendenhall, 2019

#ifndef PLUGINSAVER_HH
#define PLUGINSAVER_HH

#include "SegmentSaver.hh"
#include "ObjectFactory.hh"
#include <cassert>
#include <chrono>
using std::chrono::steady_clock;
#include <memory>
using std::shared_ptr;
using std::make_shared;
#include "StringManip.hh"
#include "libconfig.h++"
using namespace libconfig;

/// A SegmentSaver that manages several (optional) plugin SegmentSavers sharing the same file
class PluginSaver: public SegmentSaver {
public:
    /// Constructor, optionally with input filename
    PluginSaver(OutputManager* pnt, const string& nm = "PluginSaver", const string& inflName = "");
    /// Destructor
    ~PluginSaver() { for(auto p: myPlugins) delete p; }

    /// get plugin by name
    SegmentSaver* getPlugin(const string& nm) const;

    /// set printCanvas suffix (filetype)
    void setPrintSuffix(const string& sfx) override;
    /// zero out all saved histograms
    void zeroSavedHists() override;
    /// scale all saved histograms by a factor
    void scaleData(double s) override;
    /// add histograms from another SegmentSaver of the same type
    void addSegment(const SegmentSaver& S, double sc = 1.) override;

    /// optional setup at start of data loading
    void startData() override;
    /// optional event processing hook
    void processEvent() override;
    /// optional mid-processing status updates
    void checkStatus() override;
    /// optional cleanup at end of data loading
    void finishData(bool /*final*/ = true) override;
    /// perform normalization on all histograms (e.g. conversion to differential rates); should only be done once!
    void normalize() override;
    /// self-normalization before plugins
    virtual void _normalize() { SegmentSaver::normalize(); }
    /// virtual routine for generating calculated hists
    void calculateResults() override;
    /// virtual routine for generating output plots
    void makePlots() override;
    /// virtual routine for comparing to other analyzers (of this type or nullptr; meaning implementation-dependent)
    void compare(const vector<SegmentSaver*>& v) override;
    /// virtual routine to calculate incremental changes from preceding timestep
    void checkpoint(const SegmentSaver& Sprev) override;
    /// statistical test of histogram similarity
    map<string,float> compareKolmogorov(const SegmentSaver& S) const override;

    /// display plugin run time profiling; return total accounted-for time
    double displayTimeUse() const;

    /// write items to current directory or subdirectory of provided
    TDirectory* writeItems(TDirectory* d = nullptr) override;
    /// clear (delete) items
    void clearItems() override;

    /// Configure from libconfig object, dynamically loading plugins
    virtual void Configure(const Setting& cfg, bool skipUnknown = false);
    /// Configure from named .cfg file
    void LoadConfig(const string& fname);
    /// Configure, loading from input file
    void Reconfigure();

protected:
    /// load and configure plugin by class name
    void buildPlugin(const string& pname, int& copynum, const Setting& cfg, bool skipUnknown);

    map<string, SegmentSaver*> byName;      ///< available named plugins list
    vector<SegmentSaver*> myPlugins;        ///< plugins in run order
    TObjString* configstr;                  ///< configuration file contents
};

// Dynamic plugin builder requirements: return SegmentSaver*; constructor args (SegmentSaver&, const Setting&)
inline size_t pluginID(const string& cname) { return factoryID<SegmentSaver, SegmentSaver&, const Setting&>(cname,"CPB"); }

/// Base class for constructing configuration-based plugins, with parent-class recast
template <class Plug, class Base>
class ConfigPluginBuilder: public _ArgsBaseFactory<SegmentSaver, SegmentSaver&, const Setting&> {
public:
    /// Constructor, registering to list
    ConfigPluginBuilder(const string& cname): _ArgsBaseFactory<SegmentSaver, SegmentSaver&, const Setting&>(cname) {
        FactoriesIndex::index().emplace(pluginID(cname), this);
    }

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
