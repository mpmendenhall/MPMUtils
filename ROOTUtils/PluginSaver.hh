/// @file PluginSaver.hh
// -- Michael P. Mendenhall, 2019

#ifndef PLUGINSAVER_HH
#define PLUGINSAVER_HH

#include "SegmentSaver.hh"
#include "ConfigFactory.hh"
#include <chrono>
using std::chrono::steady_clock;

/// A SegmentSaver that manages several (optional) plugin SegmentSavers sharing the same file
class PluginSaver: public SegmentSaver {
public:
    /// Constructor, optionally with input filename
    explicit PluginSaver(OutputManager* pnt, const ConfigInfo_t& S, const string& _path = "PluginSaver", const string& inflName = "");
    /// Destructor
    ~PluginSaver() { for(auto p: myPlugins) delete p; }

    /// initialize, Configure()ing from new settings or restored from file
    void initialize() override;

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
    /// background subtract
    void BGSubtract(SegmentSaver& BG) override;

    /// handle datastream signals
    void signal(datastream_signal_t s) override;

    /// optional mid-processing status updates
    void checkStatus() override;
    /// divide all (scaled) distributions by run time; should only be done once!
    void normalize_runtime() override;
    /// perform normalization on all histograms
    void normalize() override;
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

protected:
    /// Configure from libconfig object, dynamically loading plugins
    virtual void Configure(SettingsQuery& S, bool skipUnknown = false);

    /// load and configure plugin by class name
    void buildPlugin(const string& pname, int& copynum, const ConfigInfo_t& cfg, bool skipUnknown);

    decltype(steady_clock::now()) ana_t0;   ///< analysis start time
    map<string, SegmentSaver*> byName;      ///< available named plugins list
    vector<SegmentSaver*> myPlugins;        ///< plugins in run order
};

/// Base class for constructing configuration-based plugins, with parent-class recast
template <class Plug, class Base>
class ConfigPluginBuilder: public _ArgsBaseFactory<SegmentSaver, SegmentSaver&, const ConfigInfo_t&> {
public:
    /// Constructor, registering to list
    explicit ConfigPluginBuilder(const string& cname): _ArgsBaseFactory<SegmentSaver, SegmentSaver&, const ConfigInfo_t&>(cname) {
        auto& idx = FactoriesIndex::indexFor<SegmentSaver, SegmentSaver&, const ConfigInfo_t&>();
        auto h = FactoriesIndex::hash(cname);
        if(idx.count(h)) throw std::logic_error("Duplicate registration of plugin named '" + cname + "'");
        idx.emplace(h, *this);
    }

    /// Re-casting plugin construction
    SegmentSaver* construct(SegmentSaver& pnt, const ConfigInfo_t& S) const override {
        auto t0 = steady_clock::now();
        auto P = new Plug(dynamic_cast<Base&>(pnt), S);
        P->tSetup += std::chrono::duration<double>(steady_clock::now()-t0).count();
        return P;
    }
};

/// Compile-time registration of dynamically-loadable plugins
#define REGISTER_PLUGIN(NAME,BASE) static ConfigPluginBuilder<NAME,BASE> the_##BASE##_##NAME##_PluginBuilder(#NAME);

#endif
