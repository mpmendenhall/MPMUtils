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
    const SegmentSaver* getPlugin(const string& nm) const;
    
    /// zero out all saved histograms
    virtual void zeroSavedHists();
    /// scale all saved histograms by a factor
    virtual void scaleData(double s);
    /// perform normalization on all histograms (e.g. conversion to differential rates); should only be done once!
    virtual void normalize();
    /// add histograms from another SegmentSaver of the same type
    virtual void addSegment(const SegmentSaver& S);
    /// virtual routine for generating output plots
    virtual void makePlots();
    /// virtual routine for generating calculated hists
    virtual void calculateResults();
    
    /// write items to file
    virtual void writeItems();
    /// clear (delete) items
    virtual void clearItems();
    
protected:
    /// build plugins appropriate for input file; call in subclass after setting up myBuilders
    virtual void buildPlugins();
    
    map<string,PluginBuilder*> myBuilders;      ///< available named plugins list
    TObjString* filePlugins;                    ///< list of plugin names from file
};

#endif
