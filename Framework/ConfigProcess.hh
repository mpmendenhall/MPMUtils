/// \file ConfigProcess.hh Base for constructing a data processing tree from a libconfig input file
// Michael P. Mendenhall, 2018

#ifndef CONFIGPROCESS_HH
#define CONFIGPROCESS_HH

#include "libconfig_readerr.hh"
#include "TreeWrap.hh"
#include "DataFrame.hh"
#include "ObjectFactory.hh"
using std::pair;

/// Convenience class for analysis run options DataFrame entry
typedef map<string,string> runInfo;

/// Base for constructing a data processing tree from a libconfig input file
class ConfigProcess: public TreeWrap<FrameSink>, public FrameSource {
public:
    /// Configure from setting (includes building child processes)
    void configure(const Setting& S);
    /// start receiving a series of data frames
    void start_data(DataFrame& F) override;
    /// process next data frame in series
    void receive(DataFrame& F, FrameSource& S) override;
    /// end series of data frames
    void end_data(DataFrame& F) override;

    /// receive frame back after processing completed
    void finished(DataFrame& F, FrameSink& S) override;

    /// Construct appropriate class from setting
    static ConfigProcess* construct(const Setting& S);

    /// indicate whether process and children keepsframe()
    bool keepsframe() const override { return _keepsframe; }

    int verbose = 0;        ///< debugging verbosity level

    /// show summary of time use
    void displayTimeSummary(int d = 0) const override;

protected:

    /// derived-module-specific configuration
    virtual void _configure(const Setting&) { }
    /// extra configuration after loading children
    virtual void postconfig(const Setting&) { }
    /// Configure a sub-module
    void add_module(const Setting& S);

    map<DataFrame*, pair<size_t,FrameSource*>> stepnum;     ///< processing step position for frames passed to children
    bool _keepsframe = false;                               ///< setting for keepsframe()

    // convenience management functions for tracking data

    /// add "borrow" tally for object from frame
    void borrow(void* o, DataFrame& F);
    /// return "borrowed" object to frame; report remaining number borrowed
    int release(void* o);
    /// function when all borrowed items are returned --- defaults to returning frame based on "stepnum"
    virtual void doneBorrowing(DataFrame& F);

    map<void*,DataFrame*> borrowed; ///< objects "borrowed" from DataFrame
    map<DataFrame*,int> nborrowed;  ///< borrowed counts from DataFrames
};

#endif
