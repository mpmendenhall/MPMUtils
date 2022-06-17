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
#include <stdexcept>

/// Base for SQLite_Helper exceptions
class SQLiteHelper_Exception: public std::runtime_error {
    /// inherit constructor
    using runtime_error::runtime_error;
};

/// Exception for when query fails to execute
class QueryFailError: public SQLiteHelper_Exception {
    /// inherit constructor
    using SQLiteHelper_Exception::SQLiteHelper_Exception;
};

/// Exception for when query fails to find expected results
class BadQueryResultError: public SQLiteHelper_Exception {
    /// inherit constructor
    using SQLiteHelper_Exception::SQLiteHelper_Exception;
};

/// Convenience wrapper for SQLite3 database interface
class SQLite_Helper {
public:
    /// Constructor
    explicit SQLite_Helper(const string& dbname, bool readonly = false, bool create = true);
    /// Constructor, adopting an open DB
    explicit SQLite_Helper(sqlite3* _db);
    /// Destructor
    virtual ~SQLite_Helper();

    /// BEGIN TRANSACTION command
    int beginTransaction(bool exclusive=false) { return (txdepth++)? SQLITE_OK : exec(exclusive? "BEGIN EXCLUSIVE TRANSACTION" : "BEGIN TRANSACTION"); }
    /// END TRANSACTION command
    int endTransaction() { return (--txdepth)? SQLITE_OK : exec("END TRANSACTION"); }

    /// set up query for use
    int setQuery(const char* qry, sqlite3_stmt*& stmt);
    /// load a cached statement
    sqlite3_stmt* loadStatement(const string& qry);
    /// retry a query until DB is available
    int busyRetry(sqlite3_stmt* stmt);
    /// run and reset a statement expecting no return values (using busyRetry); optionally, throw error if not SQLITE_OK
    int exec(sqlite3_stmt* stmt, bool checkOK = true);
    /// run a statement expecting no return values (using busyRetry); optionally, throw error if not SQLITE_OK
    int exec(const string& qry, bool checkOK = true) { return exec(loadStatement(qry), checkOK); }
    /// bind std::string to query
    static void bind_string(sqlite3_stmt* stmt, int i, const string& s);
    /// put column i string into rslt, or leave unchanged if null; return whether non-nullptr
    static bool get_string(sqlite3_stmt* stmt, unsigned int i, string& rslt);


    /// extract a vector<double> from a blob column
    void getVecBlob(vector<double>& v, sqlite3_stmt* stmt, int col);
    /// bind a vector<double> as a blob to a statement parameter
    int bindVecBlob(sqlite3_stmt* stmt, int i, const vector<double>& v);

    /// get databse file page size (bytes)
    int page_size();
    /// get databse file number of pages
    int page_count();
    /// get database file contents size (bytes)
    int db_size() { return page_size() * page_count(); }
    /// dump DB file to binary blob
    vector<char> toBlob();
    /// load contents from binary blob
    void fromBlob(const void* dat, size_t sz);

    /// use online backup calls to clone DB from other (or vice-versa)
    int backupTo(sqlite3* dbOut, bool toOther = true);

protected:
    int txdepth = 0;                        ///< depth of transaction calls
    sqlite3* db = nullptr;                  ///< database connection
    map<string, sqlite3_stmt*> statements;  ///< prepared statements awaiting deletion
};

#endif
