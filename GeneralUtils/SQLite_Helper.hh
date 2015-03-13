#ifndef SQLITE_HELPER_HH
#define SQLITE_HELPER_HH

#include "sqlite3.h"

#include <map>
using std::map;
#include <string>
using std::string;
#include <vector>
using std::vector;

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
    /// run a statement expecting no return values (using busyRetry); optionally, throw error if not SQLITE_OK
    int exec(const string& qry, bool checkOK = true);
    
    /// extract a vector<double> from a blob column
    void getVecBlob(vector<double>& v, sqlite3_stmt*& stmt, int col);
    /// bind a vector<double> as a blob to a statement parameter
    int bindVecBlob(sqlite3_stmt*& stmt, int i, const vector<double>& v);
    
    sqlite3* db = NULL;                     ///< database connection
    map<string, sqlite3_stmt*> statements;  ///< prepared statements awaiting deletion

private:
    static bool errlog_configured;          ///< whether error log output has been configured
};

#endif
