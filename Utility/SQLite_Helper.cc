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

using std::pair;

/// callback function to display SQLite3 errors
void errorLogCallback(void*, int iErrCode, const char* zMsg){
    fprintf(stderr, "SQL error (%d): %s\n", iErrCode, zMsg);
}

bool SQLite_Helper::errlog_configured = false;

SQLite_Helper::SQLite_Helper(const string& dbname, bool readonly) {
    if(!errlog_configured) {
        sqlite3_config(SQLITE_CONFIG_LOG, &errorLogCallback, nullptr);
        errlog_configured = true;
    }
    //printf("Opening SQLite3 DB '%s'...\n",dbname.c_str());
    int err = sqlite3_open_v2(dbname.c_str(), &db,
                              readonly? SQLITE_OPEN_READONLY : SQLITE_OPEN_READWRITE, nullptr);
    if(err) {
        std::runtime_error e("Failed to open DB " + dbname + " error " + sqlite3_errmsg(db));
        sqlite3_close(db);
        db = nullptr;
        throw e;
    }

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
