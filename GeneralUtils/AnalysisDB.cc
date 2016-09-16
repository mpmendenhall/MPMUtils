/// \file AnalysisDB.cc
// This file was produced under the employ of the United States Government,
// and is consequently in the PUBLIC DOMAIN, free from all provisions of
// US Copyright Law (per USC Title 17, Section 105).
//
// -- Michael P. Mendenhall, 2016

#include "AnalysisDB.hh"
#include <time.h>
#include <cassert>

AnalysisDB* AnalysisDB::myDB = nullptr;
string AnalysisDB::dbfile = "";

sqlite3_int64 AnalysisDB::createAnaRun(const string& dataname) {
    sqlite3_stmt* stmt = loadStatement("INSERT INTO analysis_runs(dataname,anatime) VALUES (?1,?2)");

    sqlite3_bind_text(stmt, 1, dataname.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 2, time(nullptr));

    busyRetry(stmt);
    sqlite3_reset(stmt);
    return sqlite3_last_insert_rowid(db);
}

sqlite3_int64 AnalysisDB::getAnaVar(const string& name, const string& unit, const string& descrip) {
    sqlite3_stmt* stmt = loadStatement("INSERT OR IGNORE INTO analysis_vars(name,unit,descrip) VALUES (?1,?2,?3)");
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, unit.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, descrip.c_str(), -1, SQLITE_STATIC);
    busyRetry(stmt);
    sqlite3_reset(stmt);

    sqlite3_stmt* stmt2 = loadStatement("SELECT rowid FROM analysis_vars WHERE name = ?1 AND descrip = ?2");
    sqlite3_bind_text(stmt2, 1, name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt2, 2, descrip.c_str(), -1, SQLITE_STATIC);
    int rc = busyRetry(stmt2);
    sqlite3_int64 var_id = (rc == SQLITE_ROW)? sqlite3_column_int64(stmt2, 0) : 0;
    sqlite3_reset(stmt2);
    assert(var_id);
    return var_id;
}

void AnalysisDB::uploadAnaResult(sqlite3_int64 run_id, sqlite3_int64 var_id, double val, double err) {
    sqlite3_stmt* stmt = loadStatement("INSERT INTO analysis_results(run,var,val,err) VALUES (?1,?2,?3,?4)");

    sqlite3_bind_int64(stmt, 1, run_id);
    sqlite3_bind_int64(stmt, 2, var_id);
    sqlite3_bind_double(stmt, 3, val);
    sqlite3_bind_double(stmt, 4, err);

    busyRetry(stmt);
    sqlite3_reset(stmt);
}
