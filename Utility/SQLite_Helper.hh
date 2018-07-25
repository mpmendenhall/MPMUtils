/// \file SQLite_Helper.hh convenience wrapper for SQLite3 database interface
// This file was produced under the employ of the United States Government,
// and is consequently in the PUBLIC DOMAIN, free from all provisions of
// US Copyright Law (per USC Title 17, Section 105).
//
// -- Michael P. Mendenhall, 2015

#ifndef SQLITE_HELPER_HH
#define SQLITE_HELPER_HH

#include "sqlite3.h"

#include <map>
using std::map;
#include <string>
using std::string;
#include <vector>
using std::vector;

/// Convenience wrapper for SQLite3 database interface
class SQLite_Helper {
public:
    /// Constructor
    SQLite_Helper(const string& dbname, bool readonly = false);
    /// Destructor
    virtual ~SQLite_Helper();

    /// BEGIN TRANSACTION command
    int beginTransaction(bool exclusive=false) { return (txdepth++)? SQLITE_OK : exec(exclusive? "BEGIN EXCLUSIVE TRANSACTION" : "BEGIN TRANSACTION"); }
    /// END TRANSACTION command
    int endTransaction() { return (--txdepth)? SQLITE_OK : exec("END TRANSACTION"); }

protected:
    /// set up query for use
    int setQuery(const char* qry, sqlite3_stmt*& stmt);
    /// load a cached statement
    sqlite3_stmt* loadStatement(const string& qry);
    /// retry a query until DB is available
    int busyRetry(sqlite3_stmt*& stmt);
    /// run a statement expecting no return values (using busyRetry); optionally, throw error if not SQLITE_OK
    int exec(const string& qry, bool checkOK = true);
    /// put column i string into rslt, or leave unchanged if null; return whether non-nullptr
    static bool get_string(sqlite3_stmt* stmt, unsigned int i, string& rslt);

    /// extract a vector<double> from a blob column
    void getVecBlob(vector<double>& v, sqlite3_stmt*& stmt, int col);
    /// bind a vector<double> as a blob to a statement parameter
    int bindVecBlob(sqlite3_stmt*& stmt, int i, const vector<double>& v);

    int txdepth = 0;                        ///< depth of transaction calls
    sqlite3* db = nullptr;                  ///< database connection
    map<string, sqlite3_stmt*> statements;  ///< prepared statements awaiting deletion

private:
    static bool errlog_configured;          ///< whether error log output has been configured
};

#endif
