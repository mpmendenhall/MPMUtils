#ifndef SQLITE_HELPER_HH
#define SQLITE_HELPER_HH

#include "sqlite3.h"

#include <map>
using std::map;
#include <string>
using std::string;

class SQLite_Helper {
public:
    /// Constructor
    SQLite_Helper(const string& dbname);
    /// Destructor
    ~SQLite_Helper();

    /// BEGIN TRANSACTION command
    int beginTransaction() { return exec("BEGIN TRANSACTION"); }
    /// END TRANSACTION command
    int endTransaction() { return exec("END TRANSACTION"); }

    
protected:    
    /// set up query for use
    int setQuery(const char* qry, sqlite3_stmt*& stmt);
    /// load a cached statement
    sqlite3_stmt* loadStatement(const string& qry);
    /// retry a query until DB is available
    int busyRetry(sqlite3_stmt*& stmt);
    /// run a statement expecting no return values (using busyRetry)
    int exec(const string& qry);

    sqlite3* db = NULL;                 ///< database connection
    static SQLite_Helper* myDB;         ///< static singleton instance
    map<string, sqlite3_stmt*> statements;   ///< prepared statements awaiting deletion

private:
    static bool errlog_configured;      ///< whether error log output has been configured
};

#endif
