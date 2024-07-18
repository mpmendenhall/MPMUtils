/// @file SQLite_Helper.cc
// This file was produced under the employ of the United States Government,
// and is consequently in the PUBLIC DOMAIN, free from all provisions of
// US Copyright Law (per USC Title 17, Section 105).
//
// -- Michael P. Mendenhall, 2015

#include "SQLite_Helper.hh"
#include "StringManip.hh"
#include "PathUtils.hh"

#include "sqlite3.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdexcept>
#include <fcntl.h>

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

SQLite_Helper::SQLite_Helper(const string& dbname, bool readonly, bool create, const string& schema) {
    if(create && readonly) throw std::logic_error("Cannot create read-only DB");

    if(memvfs_init != SQLITE_OK)
        throw SQLiteHelper_Exception("failed initialization of sqlite3 memvfs: "
        + string(sqlite3_errstr(memvfs_init)));

    if(!dbname.size()) return;
    makePath(dbname, true);

    printf("Opening SQLite3 DB '%s'...\n", dbname.c_str());
    int err = sqlite3_open_v2(dbname.c_str(), &db,
                              readonly? SQLITE_OPEN_READONLY :
                              SQLITE_OPEN_READWRITE | (create? SQLITE_OPEN_CREATE : 0),
                              nullptr);
    if(err) {
        SQLiteHelper_Exception e("Failed to open DB " + dbname + " error " + sqlite3_errmsg(db));
        sqlite3_close(db);
        db = nullptr;
        throw e;
    }

    sqlite3_busy_timeout(db, 100);

    if(schema.size()) {
        auto stmt = loadStatement("SELECT COUNT(*) FROM sqlite_master");
        busyRetry(stmt);
        bool ntables = sqlite3_column_int64(stmt, 0);
        sqlite3_reset(stmt);
        if(!ntables) {
            printf("Initializing new DB from schema file '%s'\n", schema.c_str());
            execFile(schema);
        }
    }
}

SQLite_Helper::SQLite_Helper(sqlite3* _db): db(_db) {
    if(!db) throw SQLiteHelper_Exception("SQLite_Helper initialized with nullptr DB");
    sqlite3_busy_timeout(db, 100);
}

SQLite_Helper::~SQLite_Helper() {
    if(db) {
        for(auto const& kv: statements) sqlite3_finalize(kv.second);
        sqlite3_close(db);
    }
}

int SQLite_Helper::beginTransaction(bool exclusive) {
    return (txdepth++)? SQLITE_OK : exec(exclusive? "BEGIN EXCLUSIVE TRANSACTION" : "BEGIN TRANSACTION");
}

int SQLite_Helper::endTransaction() {
    return (--txdepth)? SQLITE_OK : exec("END TRANSACTION");
}

int SQLite_Helper::setQuery(const char* qry, sqlite3_stmt*& stmt) {
    int rc;
    while((rc = sqlite3_prepare_v2(db, qry, strlen(qry), &stmt, nullptr)) == SQLITE_BUSY) {
        printf("Waiting for DB retry preparing statement...\n");
        fflush(stdout);
        usleep(500000+(rand()%500000));
    }
    if(rc != SQLITE_OK) throw QueryFailError(string("Failed query '") + qry + "' => '" + sqlite3_errmsg(db) + "'");
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

int SQLite_Helper::busyRetry(sqlite3_stmt* stmt) {
    int rc;
    while((rc = sqlite3_step(stmt)) == SQLITE_BUSY) {
        printf("Waiting for DB retry executing statement...\n");
        fflush(stdout);
        usleep(500000+(rand()%500000));
        sqlite3_reset(stmt);
    }
    return rc;
}

int SQLite_Helper::exec(sqlite3_stmt* stmt, bool checkOK) {
    int rc = busyRetry(stmt);
    sqlite3_reset(stmt);
    if(checkOK && !(rc == SQLITE_OK || rc == SQLITE_DONE)) {
        throw QueryFailError("Failed exec '" + string(sqlite3_errmsg(db)) + "'");
    }
    return rc;
}

int SQLite_Helper::execFile(const string& fname) {
    auto s = loadFileString(fname);
    char* emsg = nullptr;
    auto res = sqlite3_exec(db, s.c_str(), nullptr, nullptr, &emsg);
    if(emsg) {
        string m(emsg);
        sqlite3_free(emsg);
        throw QueryFailError("Failed execFile on '" + fname + "' with error '" + m + "'");
    }
    return res;
}

void SQLite_Helper::getVecBlob(vector<double>& v, sqlite3_stmt* stmt, int col) {
    const void* vdat = sqlite3_column_blob(stmt, col);
    if(!vdat) { v.clear(); return; }
    int nbytes = sqlite3_column_bytes(stmt, col);
    v.resize(nbytes/sizeof(double));
    memcpy(v.data(), vdat, nbytes);
}

int SQLite_Helper::bindVecBlob(sqlite3_stmt* stmt, int i, const vector<double>& v) {
    return sqlite3_bind_blob(stmt, i, v.data(), v.size()*sizeof(double), nullptr);
}

bool SQLite_Helper::get_string(sqlite3_stmt* stmt, unsigned int i, string& rslt) {
    auto s = sqlite3_column_text(stmt, i);
    if(s) rslt = string(reinterpret_cast<const char*>(s));
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
    snprintf(memname, 1024, "file:/whatever?ptr=%p&sz=0&max=%zu&freeonclose=0", v.data(), v.size());
    int err = sqlite3_open_v2(memname, &dbb, SQLITE_OPEN_READWRITE | SQLITE_OPEN_URI, "memvfs");
    if(err != SQLITE_OK) {
        SQLiteHelper_Exception e("Failed to open memdb '" + string(memname) + "', error " + sqlite3_errmsg(dbb));
        sqlite3_close(dbb);
        throw e;
    }
    SQLite_Helper H(dbb);
    H.exec("PRAGMA journal_mode=OFF", false);
    backupTo(dbb);
    if(H.db_size() != (int)v.size()) throw SQLiteHelper_Exception("Unexpected database binary size");
    return v;
}

void SQLite_Helper::bind_string(sqlite3_stmt* stmt, int i, const string& s) {
    sqlite3_bind_text(stmt, i, s.c_str(), s.size(), SQLITE_TRANSIENT);
}

void SQLite_Helper::fromBlob(const void* dat, size_t sz) {
    sqlite3* dbb = nullptr;
    char memname[1024];
    snprintf(memname, 1024, "file:/whatever?ptr=%p&sz=%zu&max=%zu&freeonclose=0", dat, sz, sz);
    int err = sqlite3_open_v2(memname, &dbb, SQLITE_OPEN_READWRITE | SQLITE_OPEN_URI, "memvfs");
    if(err != SQLITE_OK) {
        SQLiteHelper_Exception e("Failed to open memdb '" + string(memname) + "', error " + sqlite3_errmsg(dbb));
        sqlite3_close(dbb);
        throw e;
    }
    SQLite_Helper H(dbb);
    if(H.db_size() != (int)sz) throw std::logic_error("Unexpected database binary size");

    backupTo(dbb, false);
}
