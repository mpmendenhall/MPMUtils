#include "SQLite_Helper.hh"
#include "SMExcept.hh"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

using std::pair;

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
    sqlite3_busy_timeout(db, 1001);
}

SQLite_Helper::~SQLite_Helper() {
    if(db) {
        for(auto it = statements.begin(); it != statements.end(); it++) sqlite3_finalize(it->second);
        sqlite3_close(db);
    }
}

int SQLite_Helper::setQuery(const char* qry, sqlite3_stmt*& stmt) {
    int rc;
    while((rc = sqlite3_prepare_v2(db, qry, strlen(qry), &stmt, NULL)) == SQLITE_BUSY) {
        printf("Waiting for DB retry preparing statement...\n");
        usleep(500000);
    }
    if(rc != SQLITE_OK) {
        SMExcept e("failed_query");
        e.insert("message",sqlite3_errmsg(db));
        throw(e);
    }
    return rc;
}

sqlite3_stmt* SQLite_Helper::loadStatement(const string& qry) {
    auto it = statements.find(qry);
    if(it != statements.end()) return it->second;
    sqlite3_stmt* stmt = NULL;
    setQuery(qry.c_str(), stmt);
    statements.insert(pair<string,sqlite3_stmt*>(qry,stmt));
    return stmt;
}

int SQLite_Helper::busyRetry(sqlite3_stmt*& stmt) {
    int rc;
    while((rc = sqlite3_step(stmt)) == SQLITE_BUSY) { printf("Waiting for DB retry executing statement...\n"); }
    return rc;
}

int SQLite_Helper::exec(const string& qry) {
    sqlite3_stmt* stmt = loadStatement(qry);
    int rc = busyRetry(stmt);
    sqlite3_reset(stmt);
    return rc;
}
