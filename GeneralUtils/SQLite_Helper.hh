#ifndef SQLITE_HELPER_HH
#define SQLITE_HELPER_HH

#include "sqlite3.h"

#include <vector>
using std::vector;
#include <string>
using std::string;

class SQLite_Helper {
public:
    /// Constructor
    SQLite_Helper(const string& dbname);
    /// Destructor
    ~SQLite_Helper();

protected:    
    /// set up query for use
    int setQuery(const char* qry, sqlite3_stmt*& stmt);
    /// retry a query until DB is available
    int busyRetry(sqlite3_stmt*& stmt);
    
    sqlite3* db = NULL;                 ///< database connection
    static SQLite_Helper* myDB;         ///< static singleton instance
    vector<sqlite3_stmt*> statements;   ///< prepared statements awaiting deletion

private:
    static bool errlog_configured;      ///< whether error log output has been configured
};

#endif
