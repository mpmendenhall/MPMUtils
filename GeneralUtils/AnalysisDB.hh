/// \file AnalysisDB.hh Interface to database of analysis results
// -- Michael P. Mendenhall, LLNL 2019

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

    static string dbfile;   ///< database file location

    /// Information on an analysis variable
    struct AnaVar {
        string name;        ///< name
        string unit;        ///< units
        string descrip;     ///< description
    };

    /// Struct for holding analysis results for later deferred upload
    struct AnaResult: public AnaVar {
        /// Constructor
        AnaResult(const string& nm, const string& u, const string& dsc, double v, double e):
        AnaVar{nm,u,dsc}, val(v), err(e) { }
        /// Constructor for text value
        AnaResult(const string& nm, const string& u, const string& dsc, const string& v):
        AnaVar{nm,u,dsc}, val(0), err(0), xval(v) { }
        /// Display contents
        void display() const;

        double val;         ///< value
        double err;         ///< uncertainty on value
        string xval;        ///< text value (supercedes val/err)
    };

    /// create analysis dataset identifier
    sqlite3_int64 createAnaData(const string& dataname);
    /// get (or create) analysis variable identifier
    sqlite3_int64 getAnaVar(const string& name, const string& unit, const string& descrip);
    /// get (or create) analysis variable identifier
    sqlite3_int64 getAnaVar(const AnaVar& V) { return getAnaVar(V.name,V.unit,V.descrip); }
    /// create identifier for result
    sqlite3_int64 getResultID(sqlite3_int64 data_id, sqlite3_int64 var_id);

    /// upload numerical analysis result
    void uploadAnaResult(sqlite3_int64 result_id, double val, double err);
    /// upload numerical analysis result
    void uploadAnaResult(sqlite3_int64 data_id, sqlite3_int64 var_id, double val, double err) {
        uploadAnaResult(getResultID(data_id,var_id), val, err);
    }

    /// upload text analysis result
    void uploadAnaResult(sqlite3_int64 result_id, const string& val);
    /// upload text analysis result
    void uploadAnaResult(sqlite3_int64 data_id, sqlite3_int64 var_id, const string& val) {
        uploadAnaResult(getResultID(data_id,var_id), val);
    }

protected:
    /// Constructor
    AnalysisDB(): SQLite_Helper(dbfile) { }

    static AnalysisDB* myDB;    ///< singleton instance of DB connection
    sqlite3_int64 code_id();    ///< create identifier for code this is run in
};

#endif
