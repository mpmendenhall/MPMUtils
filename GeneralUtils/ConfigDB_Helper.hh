/// \file ConfigDB_Helper.hh SQLite3 configuration database interface
// This file was produced under the employ of the United States Government,
// and is consequently in the PUBLIC DOMAIN, free from all provisions of
// US Copyright Law (per USC Title 17, Section 105).
//
// -- Michael P. Mendenhall, 2016

#ifndef CONFIGDB_HELPER_HH
#define CONFIGDB_HELPER_HH

#include "SQLite_Helper.hh"
#include "Stringmap.hh"
#include "SMFile.hh"

/// Interface to SQLite3 "configuration database" schema
class ConfigDB_Helper: public SQLite_Helper {
public:
    /// Constructor
    ConfigDB_Helper(const string& dbname): SQLite_Helper(dbname) { }
    
    /// Get named configuration as Stringmap
    Stringmap getConfig(const string& family, const string& name);
    /// Get configuration by ID number
    Stringmap getConfig(sqlite3_int64 cid);
    /// Get all configurations in family
    SMFile getConfigs(const string& family);
};

#endif
