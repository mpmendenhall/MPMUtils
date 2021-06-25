/// \file AnalysisDB.cc
// This file was produced under the employ of the United States Government,
// and is consequently in the PUBLIC DOMAIN, free from all provisions of
// US Copyright Law (per USC Title 17, Section 105).
//
// -- Michael P. Mendenhall, 2016

#include "AnalysisDB.hh"
#include "to_str.hh"
#include "AnaGlobals.hh"
#include "GetEnv.hh"
#include "PathUtils.hh"
#include <time.h>
#include <functional>

AnalysisDB* AnalysisDB::myDB = nullptr;

string ADBfile() {
    string dbvar = PROJ_ENV_PFX()+"_ANADB";
    string res = "$"+dbvar;
    auto dbfile = optionalGlobalArg("AnaDB", res, "Analysis DB file")? res : getEnv(dbvar);

    if(!fileExists(dbfile)) {
        printf("Initializing new AnaDB at '%s'\n", dbfile.c_str());
        makePath(dbfile, true);
        string cmd = "sqlite3 '" + dbfile + "' < "+getEnv(PROJ_ENV_PFX()+"_CODE",true)+"/Utility/AnalysisDB_Schema.sql";
        int err = system(cmd.c_str());
        if(err) throw std::runtime_error("Bad AnaDB path '"+dbfile+"'");
    }
    return dbfile;
}

AnalysisDB::AnalysisDB(): SQLite_Helper(ADBfile()) { }

sqlite3_int64 AnalysisDB::createAnaRun(const string& dataname) {
    auto t = time(nullptr);
    auto runid = std::hash<string>{}(dataname + to_str(t));
    auto stmt = loadStatement("INSERT INTO analysis_runs(run_id,dataname,anatime) VALUES (?1,?2,?3)");
    sqlite3_bind_int64(stmt, 1, runid);
    sqlite3_bind_text(stmt, 2, dataname.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 3, t);
    busyRetry(stmt);
    sqlite3_reset(stmt);

    auto stmt2 = loadStatement("SELECT run_id FROM analysis_runs WHERE rowid = last_insert_rowid()");
    busyRetry(stmt2);
    auto r = sqlite3_column_int64(stmt2, 0);
    sqlite3_reset(stmt2);

    return r;
}

sqlite3_int64 AnalysisDB::getAnaVar(const string& name, const string& unit, const string& descrip) {
    auto stmt = loadStatement("INSERT OR IGNORE INTO analysis_vars(var_id,name,unit,descrip) VALUES (?1,?2,?3,?4)");
    sqlite3_bind_int64(stmt, 1, std::hash<string>{}(name));
    sqlite3_bind_text(stmt, 2, name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, unit.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, descrip.c_str(), -1, SQLITE_STATIC);
    busyRetry(stmt);
    sqlite3_reset(stmt);

    auto stmt2 = loadStatement("SELECT var_id FROM analysis_vars WHERE name = ?1 AND descrip = ?2");
    sqlite3_bind_text(stmt2, 1, name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt2, 2, descrip.c_str(), -1, SQLITE_STATIC);
    busyRetry(stmt2);
    sqlite3_int64 var_id = sqlite3_column_int64(stmt2, 0);
    sqlite3_reset(stmt2);
    return var_id;
}

void AnalysisDB::uploadAnaResult(sqlite3_int64 run_id, sqlite3_int64 var_id, double val, double err) {
    auto stmt = loadStatement("INSERT INTO analysis_results(run_id,var_id,val,err) VALUES (?1,?2,?3,?4)");

    sqlite3_bind_int64(stmt, 1, run_id);
    sqlite3_bind_int64(stmt, 2, var_id);
    sqlite3_bind_double(stmt, 3, val);
    sqlite3_bind_double(stmt, 4, err);

    busyRetry(stmt);
    sqlite3_reset(stmt);
}

void AnalysisDB::uploadAnaResult(sqlite3_int64 run_id, sqlite3_int64 var_id, const string& val) {
    auto stmt = loadStatement("INSERT INTO analysis_xresults(run_id,var_id,val) VALUES (?1,?2,?3)");

    sqlite3_bind_int64(stmt, 1, run_id);
    sqlite3_bind_int64(stmt, 2, var_id);
    sqlite3_bind_text(stmt, 3, val.c_str(), -1, SQLITE_STATIC);

    busyRetry(stmt);
    sqlite3_reset(stmt);
}

void AnaResult::display() const {
    printf("%s [%s]:\t", name.c_str(), descrip.c_str());
    if(xval.size()) printf("%s [%s]\n", xval.c_str(), unit.c_str());
    else printf("%g ~ %g %s\n", val, err, unit.c_str());
}
