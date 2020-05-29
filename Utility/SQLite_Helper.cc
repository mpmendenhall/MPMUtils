/// \file SQLite_Helper.cc
// This file was produced under the employ of the United States Government,
// and is consequently in the PUBLIC DOMAIN, free from all provisions of
// US Copyright Law (per USC Title 17, Section 105).
//
// -- Michael P. Mendenhall, 2015

#include "SQLite_Helper.hh"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdexcept>
#include <fcntl.h>

using std::pair;

/// callback function to display SQLite3 errors
void errorLogCallback(void*, int iErrCode, const char* zMsg){
    printf("SQL error (%d): %s\n", iErrCode, zMsg);
}


extern "C" {
    int sqlite3_memvfs_init(sqlite3 *db, char **pzErrMsg, const sqlite3_api_routines *pApi);
}
//const int errlog_is_configured = sqlite3_config(SQLITE_CONFIG_LOG, &errorLogCallback, nullptr);
//if(errlog_is_configured != SQLITE_OK) throw std::logic_error("sqlite error logging misconfigured");


static int memvfs_init = sqlite3_auto_extension((void(*)(void)) &sqlite3_memvfs_init);

SQLite_Helper::SQLite_Helper(const string& dbname, bool readonly, bool create) {
    if(memvfs_init != SQLITE_OK)
        throw std::logic_error("failed initialization of sqlite3 memvfs: "
        + string(sqlite3_errstr(memvfs_init)));

    printf("Opening SQLite3 DB '%s'...\n", dbname.c_str());
    int err = sqlite3_open_v2(dbname.c_str(), &db,
                              readonly? SQLITE_OPEN_READONLY :
                              SQLITE_OPEN_READWRITE | (create? SQLITE_OPEN_CREATE : 0),
                              nullptr);
    if(err) {
        std::runtime_error e("Failed to open DB " + dbname + " error " + sqlite3_errmsg(db));
        sqlite3_close(db);
        db = nullptr;
        throw e;
    }

    sqlite3_busy_timeout(db, 100);
}

SQLite_Helper::SQLite_Helper(sqlite3* _db): db(_db) {
    if(!db) throw std::logic_error("SQLite_Helper initialized with nullptr DB");
    sqlite3_busy_timeout(db, 100);
}

SQLite_Helper::~SQLite_Helper() {
    if(db) {
        for(auto const& kv: statements) sqlite3_finalize(kv.second);
        sqlite3_close(db);
    }
}

int SQLite_Helper::setQuery(const char* qry, sqlite3_stmt*& stmt) {
    int rc;
    while((rc = sqlite3_prepare_v2(db, qry, strlen(qry), &stmt, nullptr)) == SQLITE_BUSY) {
        printf("Waiting for DB retry preparing statement...\n");
        fflush(stdout);
        usleep(500000+(rand()%500000));
    }
    if(rc != SQLITE_OK) throw std::runtime_error(string("Failed query '") + qry + "' => '" + sqlite3_errmsg(db) + "'");
    return rc;
}

sqlite3_stmt* SQLite_Helper::loadStatement(const string& qry) {
    auto it = statements.find(qry);
    if(it != statements.end()) return it->second;
    sqlite3_stmt* stmt = nullptr;
    setQuery(qry.c_str(), stmt);
    statements.emplace(qry,stmt);
    return stmt;
}

int SQLite_Helper::busyRetry(sqlite3_stmt*& stmt) {
    int rc;
    while((rc = sqlite3_step(stmt)) == SQLITE_BUSY) {
        printf("Waiting for DB retry executing statement...\n");
        fflush(stdout);
        usleep(500000+(rand()%500000));
        sqlite3_reset(stmt);
    }
    return rc;
}

int SQLite_Helper::exec(const string& qry, bool checkOK) {
    auto stmt = loadStatement(qry);
    int rc = busyRetry(stmt);
    sqlite3_reset(stmt);
    if(checkOK && !(rc == SQLITE_OK || rc == SQLITE_DONE)) {
        throw std::runtime_error(string("Failed exec '") + qry + "' => '" + sqlite3_errmsg(db) + "'");
    }
    return rc;
}

void SQLite_Helper::getVecBlob(vector<double>& v, sqlite3_stmt*& stmt, int col) {
    const void* vdat = sqlite3_column_blob(stmt, col);
    if(!vdat) { v.clear(); return; }
    int nbytes = sqlite3_column_bytes(stmt, col);
    v.resize(nbytes/sizeof(double));
    memcpy(v.data(), vdat, nbytes);
}

int SQLite_Helper::bindVecBlob(sqlite3_stmt*& stmt, int i, const vector<double>& v) {
    return sqlite3_bind_blob(stmt, i, v.data(), v.size()*sizeof(double), nullptr);
}

bool SQLite_Helper::get_string(sqlite3_stmt* stmt, unsigned int i, string& rslt) {
    const unsigned char* s = sqlite3_column_text(stmt, i);
    if(s) rslt = string((const char*)s);
    return s;
}

int SQLite_Helper::page_size() {
    auto stmt = loadStatement("PRAGMA page_size");
    busyRetry(stmt);
    auto r = sqlite3_column_int(stmt, 0);
    sqlite3_reset(stmt);
    return r;
}

int SQLite_Helper::page_count() {
    auto stmt = loadStatement("PRAGMA page_count");
    busyRetry(stmt);
    auto r = sqlite3_column_int(stmt, 0);
    sqlite3_reset(stmt);
    return r;
}

int SQLite_Helper::backupTo(sqlite3* dbOut, bool toOther) {
    sqlite3_backup* pBackup = sqlite3_backup_init(toOther? dbOut : db, "main", toOther? db : dbOut, "main");
    if(pBackup) {
        (void)sqlite3_backup_step(pBackup, -1); // performs backup operation
        (void)sqlite3_backup_finish(pBackup);   // releases pBackup
    }
    return sqlite3_errcode(toOther? dbOut : db);
}

vector<char> SQLite_Helper::toBlob() {
    vector<char> v(db_size());

    sqlite3* dbb = nullptr;
    char memname[1024];
    sprintf(memname, "file:/whatever?ptr=%p&sz=0&max=%zu&freeonclose=0", v.data(), v.size());
    int err = sqlite3_open_v2(memname, &dbb, SQLITE_OPEN_READWRITE | SQLITE_OPEN_URI, "memvfs");
    if(err != SQLITE_OK) {
        std::runtime_error e("Failed to open memdb '" + string(memname) + "', error " + sqlite3_errmsg(dbb));
        sqlite3_close(dbb);
        throw e;
    }
    SQLite_Helper H(dbb);
    H.exec("PRAGMA journal_mode=OFF", false);
    backupTo(dbb);
    if(H.db_size() != (int)v.size()) throw std::logic_error("Unexpected database binary size");
    return v;
}

void SQLite_Helper::fromBlob(const void* dat, size_t sz) {
    sqlite3* dbb = nullptr;
    char memname[1024];
    sprintf(memname, "file:/whatever?ptr=%p&sz=%zu&max=%zu&freeonclose=0", dat, sz, sz);
    int err = sqlite3_open_v2(memname, &dbb, SQLITE_OPEN_READWRITE | SQLITE_OPEN_URI, "memvfs");
    if(err != SQLITE_OK) {
        std::runtime_error e("Failed to open memdb '" + string(memname) + "', error " + sqlite3_errmsg(dbb));
        sqlite3_close(dbb);
        throw e;
    }
    SQLite_Helper H(dbb);
    if(H.db_size() != (int)sz) throw std::logic_error("Unexpected database binary size");

    backupTo(dbb, false);
}
