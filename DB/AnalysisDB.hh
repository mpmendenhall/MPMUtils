/// @file AnalysisDB.hh Interface to database of analysis results
// This file was produced under the employ of the United States Government,
// and is consequently in the PUBLIC DOMAIN, free from all provisions of
// US Copyright Law (per USC Title 17, Section 105).
//
// -- Michael P. Mendenhall, 2016

#ifndef ANALYSISDB_HH
#define ANALYSISDB_HH

#include "SQLite_Helper.hh"

/// Calibration database interface
class AnalysisDB: public SQLite_Helper {
public:
    /// get singleton instance
    static AnalysisDB& DB() { return *(myDB? myDB : myDB = new AnalysisDB()); }
    /// close and delete instance
    static void closeDB() { if(myDB) { delete myDB; myDB = nullptr; } }

    /// DB identifier for run
    enum anarun_id_t: sqlite3_int64 { };
    /// DB identifier for variable
    enum anavar_id_t: sqlite3_int64 { };

    /// create analysis run identifier
    anarun_id_t createAnaRun(const string& dataname);
    /// get (or create) analysis variable identifier
    anavar_id_t getAnaVar(const string& name, const string& unit, const string& descrip);
    /// upload analysis result
    void uploadAnaResult(anarun_id_t run_id, anavar_id_t var_id, double val, double err);
    /// upload text analysis result
    void uploadAnaResult(anarun_id_t run_id, anavar_id_t var_id, const string& val);

protected:
    /// Constructor
    AnalysisDB();

    static AnalysisDB* myDB;    ///< singleton instance of DB connection
};

/// Struct for holding analysis results until upload
struct AnaResult {
    /// Constructor
    AnaResult(const string& nm, const string& u, const string& dsc, double v, double e):
    name(nm), unit(u), descrip(dsc), val(v), err(e) { }
    /// Constructor for text value
    AnaResult(const string& nm, const string& u, const string& dsc, const string& v):
    name(nm), unit(u), descrip(dsc), val(0), err(0), xval(v) { }
    /// Display contents
    void display() const;

    string name;        ///< name
    string unit;        ///< units
    string descrip;     ///< description
    double val;         ///< value
    double err;         ///< uncertainty on value
    string xval;        ///< text value (supercedes val/err)
};

#endif
