/// \file TObjCollector.hh
// This file was produced under the employ of the United States Government,
// and is consequently in the PUBLIC DOMAIN, free from all provisions of
// US Copyright Law (per USC Title 17, Section 105).
//
// -- Michael P. Mendenhall, 2015

#ifndef TOBJCOLLECTOR_HH
#define TOBJCOLLECTOR_HH

#include <TObject.h>
#include <TNamed.h>
#include <TDirectory.h>
#include <TH1.h>
#include <TH1F.h>
#include <TH2F.h>

#include <string>
using std::string;
#include <vector>
using std::vector;
#include <map>
using std::map;

/// collection of saved TObjects
class TObjCollector {
public:
    /// destructor
    virtual ~TObjCollector() { deleteAll(); }

    /// write items to currently open directory, or specified; return directory written to
    virtual TDirectory* writeItems(TDirectory* d = nullptr);
    /// clear (delete) items
    void deleteAll();
    /// register a named ROOT object for output (and eventual deletion)
    virtual TNamed* addObject(TNamed* o);
    /// register object to deletion list (not written to file)
    virtual TObject* addDeletable(TObject* o);
    /// register an anonymous ROOT object under the specified name
    virtual TObject* addWithName(TObject* o, const string& name);

    /// generate a TH1F registered with output objects list
    TH1F* registeredTH1F(string hname, string htitle, unsigned int nbins, float x0, float x1);
    /// generate a TH1D registered with output objects list
    TH1D* registeredTH1D(string hname, string htitle, unsigned int nbins, float x0, float x1);
    /// generate a TH2F registered with output objects list
    TH2F* registeredTH2F(string hname, string htitle, unsigned int nbinsx, float x0, float x1, unsigned int nbinsy, float y0, float y1);

    vector<TNamed*> namedItems;         ///< objects with their own names; held until deleted
    map<string,TObject*> anonItems;     ///< objects assigned a name; held until deleted.
    vector<TObject*> deleteItems;       ///< other objects never written to file
};

#endif
