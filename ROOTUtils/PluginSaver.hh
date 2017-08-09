/// \file PluginSaver.hh
// This file was produced under the employ of the United States Government,
// and is consequently in the PUBLIC DOMAIN, free from all provisions of
// US Copyright Law (per USC Title 17, Section 105).
//
// -- Michael P. Mendenhall, 2015

#ifndef PLUGINSAVER_HH
#define PLUGINSAVER_HH

#include "SegmentSaver.hh"
#include <cassert>
#include <chrono>
using std::chrono::steady_clock;
#include <memory>
using std::shared_ptr;
using std::make_shared;

/// Base class for optionally constructing SegmentSaver plugins "on demand"
class PluginBuilder {
public:
    /// Destructor
    virtual ~PluginBuilder() { }

    /// instantiate plugin SegmentSaver
    virtual void makePlugin(SegmentSaver* pnt) = 0;

    shared_ptr<SegmentSaver> thePlugin = nullptr; ///< instantiated plugin
};

/// Simple templatized PluginBuilder, checking for correct parent type
template <class Base, class Plug>
class RecastPluginBuilder: public PluginBuilder {
public:
    /// Constructor
    RecastPluginBuilder() { }
    /// Re-casting plugin construction
    void makePlugin(SegmentSaver* pnt) override {
        auto PBase = dynamic_cast<Base*>(pnt);
        assert(PBase);
        thePlugin = _makePlugin(PBase);
    }
    /// Create appropriate plugin type
    virtual shared_ptr<SegmentSaver> _makePlugin(Base* PBase) {
        auto t0 = steady_clock::now();
        auto p = make_shared<Plug>(PBase);
        p->tSetup += std::chrono::duration<double>(steady_clock::now()-t0).count();
        return p;
    }
};

/// A SegmentSaver that manages several (optional) plugin SegmentSavers sharing the same file
class PluginSaver: public SegmentSaver {
public:
    /// Constructor, optionally with input filename
    PluginSaver(OutputManager* pnt, const string& nm = "PluginSaver", const string& inflName = "");

    /// get plugin by name
    shared_ptr<SegmentSaver> getPlugin(const string& nm) const;

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
    /// optional cleanup at end of data loading
    void finishData() override;
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

    /// display plugin run time profiling; return total accounted-for time
    double displayTimeUse() const;

    /// write items to current directory or subdirectory of provided
    TDirectory* writeItems(TDirectory* d = nullptr) override;
    /// clear (delete) items
    void clearItems() override;

protected:
    /// build plugins appropriate for input file; call in subclass after setting up myBuilders
    virtual void buildPlugins();

    map<string, shared_ptr<PluginBuilder>> myBuilders;  ///< available named plugins list
    TObjString* filePlugins;                            ///< list of plugin names from file
};

#endif
