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

/// Base class for optionally constructing SegmentSaver plugins "on demand"
class PluginBuilder {
public:
    /// instantiate plugin SegmentSaver
    virtual void makePlugin(SegmentSaver* pnt) = 0;
    
    SegmentSaver* thePlugin = NULL;     ///< instantiated plugin (not memory managed here)
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
    /// Subclass me!
    virtual SegmentSaver* _makePlugin(Base* PBase) { return new Plug(PBase); }
};


/// A SegmentSaver that manages several (optional) plugin SegmentSavers sharing the same file
class PluginSaver: public SegmentSaver {
public:
    /// Constructor, optionally with input filename
    PluginSaver(OutputManager* pnt, const string& nm = "PluginSaver", const string& inflName = "");
    /// Destructor
    ~PluginSaver();
    
    /// get plugin by name; NULL if not available
    SegmentSaver* getPlugin(const string& nm) const;
    
    /// zero out all saved histograms
    void zeroSavedHists() override;
    /// scale all saved histograms by a factor
    void scaleData(double s) override;
    /// perform normalization on all histograms (e.g. conversion to differential rates); should only be done once!
    void normalize() override;
    /// self-normalization before plugins
    virtual void _normalize() { SegmentSaver::normalize(); }
    /// add histograms from another SegmentSaver of the same type
    void addSegment(const SegmentSaver& S, double sc = 1.) override;
    /// virtual routine for generating output plots
    void makePlots() override;
    /// virtual routine for generating calculated hists
    void calculateResults() override;
    /// virtual routine for comparing to other analyzers (of this type or NULL; meaning implementation-dependent)
    void compare(const vector<SegmentSaver*>& v) override;
    
    /// write items to current directory or subdirectory of provided
    TDirectory* writeItems(TDirectory* d = NULL) override;
    /// clear (delete) items
    void clearItems() override;
    
protected:
    /// build plugins appropriate for input file; call in subclass after setting up myBuilders
    virtual void buildPlugins();
    
    map<string,PluginBuilder*> myBuilders;      ///< available named plugins list
    TObjString* filePlugins;                    ///< list of plugin names from file
};

#endif
