/// @file TObjCollector.hh
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

    /// register a named ROOT object (using its own name) for output (and eventual deletion)
    void _addObject(TNamed* o);
    /// register an anonymous ROOT object under the specified name
    void _addObject(TObject* o, const string& name);

    /// convenience pass-through wrapper
    template<class T>
    T* addObject(T* o) { _addObject(o); return o; }
    /// convenience pass-through wrapper
    template<class T>
    T* addObject(T* o, const string& name) { _addObject(o, name); return o; }

    /// register object to deletion list (not written to file)
    TObject* addDeletable(TObject* o);

    map<string, TObject*> namedItems;    ///< objects to save; owned by this class
    vector<TObject*> deleteItems;       ///< other objects never written to file
};

#endif
