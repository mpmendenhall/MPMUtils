/// \file PluginSaver.hh
// This file was produced under the employ of the United States Government,
// and is consequently in the PUBLIC DOMAIN, free from all provisions of
// US Copyright Law (per USC Title 17, Section 105).
// 
// -- Michael P. Mendenhall, 2015

#ifndef PLUGINSAVER_HH
#define PLUGINSAVER_HH

#include "SegmentSaver.hh"

/// Base class for optionally constructing SegmentSaver plugins "on demand"
class PluginBuilder {
public:
    /// instantiate plugin SegmentSaver
    virtual void makePlugin(OutputManager* pnt, const string& inflName = "") = 0;
    
    SegmentSaver* thePlugin = NULL;     ///< instantiated plugin (not memory managed here)
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
    virtual void zeroSavedHists() override;
    /// scale all saved histograms by a factor
    virtual void scaleData(double s) override;
    /// perform normalization on all histograms (e.g. conversion to differential rates); should only be done once!
    virtual void normalize() override;
    /// add histograms from another SegmentSaver of the same type
    virtual void addSegment(const SegmentSaver& S) override;
    /// virtual routine for generating output plots
    virtual void makePlots() override;
    /// virtual routine for generating calculated hists
    virtual void calculateResults() override;
    /// virtual routine for comparing to other analyzers (of this type or NULL; meaning implementation-dependent)
    virtual void compare(const vector<SegmentSaver*>& v) override;
    
    /// write items to file
    virtual void writeItems() override;
    /// clear (delete) items
    virtual void clearItems() override;
    
protected:
    /// build plugins appropriate for input file; call in subclass after setting up myBuilders
    virtual void buildPlugins();
    
    map<string,PluginBuilder*> myBuilders;      ///< available named plugins list
    TObjString* filePlugins;                    ///< list of plugin names from file
};

#endif
