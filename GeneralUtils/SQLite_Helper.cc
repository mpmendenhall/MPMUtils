#include "SQLite_Helper.hh"
#include "SMExcept.hh"
#include <stdio.h>
#include <string.h>

void errorLogCallback(void*, int iErrCode, const char* zMsg){
    fprintf(stderr, "SQL error (%d): %s\n", iErrCode, zMsg);
}

bool SQLite_Helper::errlog_configured = false;

SQLite_Helper::SQLite_Helper(const string& dbname) {
    if(!errlog_configured) {
        sqlite3_config(SQLITE_CONFIG_LOG, &errorLogCallback, NULL);
        errlog_configured = true;
    }
    printf("Opening SQLite3 DB '%s'...\n",dbname.c_str());
    int err = sqlite3_open(dbname.c_str(), &db);
    if(err) {
        SMExcept e("failed_db_open");
        e.insert("dbname",dbname);
        e.insert("message",sqlite3_errmsg(db));
        sqlite3_close(db);
        db = NULL;
        throw e;
    }
}

SQLite_Helper::~SQLite_Helper() {
    if(db) {
        for(auto it = statements.begin(); it != statements.end(); it++) sqlite3_finalize(*it);
        sqlite3_close(db);
    }
}

int SQLite_Helper::setQuery(const char* qry, sqlite3_stmt*& stmt) {
    int rc = sqlite3_prepare_v2(db, qry, strlen(qry), &stmt, NULL);
    if(rc != SQLITE_OK) {
        SMExcept e("failed_query");
        e.insert("message",sqlite3_errmsg(db));
        throw(e);
    }
    return rc;
}
