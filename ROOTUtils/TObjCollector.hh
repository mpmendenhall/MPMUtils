/// \file TObjCollector.hh
// This file was produced under the employ of the United States Government,
// and is consequently in the PUBLIC DOMAIN, free from all provisions of
// US Copyright Law (per USC Title 17, Section 105).
// 
// -- Michael P. Mendenhall, 2015

#ifndef TOBJCOLLECTOR_HH
#define TOBJCOLLECTOR_HH

#include <TFile.h>
#include <TObject.h>
#include <TNamed.h>

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
    virtual ~TObjCollector() { clearItems(); }
    
    /// write items to file
    virtual void writeItems();
    /// clear (delete) items
    void clearItems();
    /// register a named ROOT object for output (and eventual deletion)
    virtual TNamed* addObject(TNamed* o);
    /// register object to deletion list (not written to file)
    virtual TObject* addDeletable(TObject* o);
    /// register an anonymous ROOT object under the specified name
    virtual TObject* addWithName(TObject* o, const string& name);
    
    vector<TNamed*> namedItems;         ///< objects with their own names; held until deleted
    map<string,TObject*> anonItems;     ///< objects assigned a name; held until deleted.     
    vector<TObject*> deleteItems;       ///< other objects never written to file
};

#endif
