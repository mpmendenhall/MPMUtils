/// @file AnalysisDB.cc
// This file was produced under the employ of the United States Government,
// and is consequently in the PUBLIC DOMAIN, free from all provisions of
// US Copyright Law (per USC Title 17, Section 105).
//
// -- Michael P. Mendenhall, 2016

#include "AnalysisDB.hh"
#include "to_str.hh"
#include "GlobalArgs.hh"
#include "GetEnv.hh"
#include "PathUtils.hh"
#include "TermColor.hh"
#include <time.h>
#include <functional>
#include <stdio.h>

AnalysisDB* AnalysisDB::myDB = nullptr;

string ADBfile() {
    string dbvar = PROJ_ENV_PFX()+"_ANADB";
    string res = "$"+dbvar;
    auto dbfile = optionalGlobalArg("AnaDB", res, "Analysis DB file")? res : getEnv(dbvar);

    if(!dbfile.size()) {
        printf(TERMFG_RED "Warning: no $%s_ANADB file specified" TERMSGR_RESET "\n", PROJ_ENV_PFX().c_str());
        return "";
    }

    return dbfile;
}

AnalysisDB::AnalysisDB():
SQLite_Helper(ADBfile(), false, true,
              getEnv(PROJ_ENV_PFX()+"_CODE",true)+"/DB/AnalysisDB_Schema.sql") {
    exec("PRAGMA foreign_keys = ON");
}

AnalysisDB::anarun_id_t AnalysisDB::createAnaRun(const string& dataname) {
    auto t = time(nullptr);
    auto runid = anarun_id_t(std::hash<string>{}(dataname + to_str(t)));
    auto stmt = loadStatement("INSERT INTO analysis_runs(run_id,dataname,anatime) VALUES (?1,?2,?3)");
    sqlite3_bind_int64(stmt, 1, runid);
    bind_string(stmt, 2, dataname);
    sqlite3_bind_double(stmt, 3, t);
    busyRetry(stmt);
    sqlite3_reset(stmt);

    return runid;
}

AnalysisDB::anavar_id_t AnalysisDB::getAnaVar(const string& name, const string& unit, const string& descrip) {
    auto stmt = loadStatement("INSERT OR IGNORE INTO analysis_vars(var_id,name,unit,descrip) VALUES (?1,?2,?3,?4)");
    auto varid = anavar_id_t(std::hash<string>{}(name));
    sqlite3_bind_int64(stmt, 1, varid);
    bind_string(stmt, 2, name);
    bind_string(stmt, 3, unit);
    bind_string(stmt, 4, descrip);
    busyRetry(stmt);
    sqlite3_reset(stmt);

    return varid;
}

void AnalysisDB::uploadAnaResult(anarun_id_t run_id, anavar_id_t var_id, double val, double err) {
    auto stmt = loadStatement("INSERT INTO analysis_results(run_id,var_id,val,err) VALUES (?1,?2,?3,?4)");

    sqlite3_bind_int64(stmt, 1, run_id);
    sqlite3_bind_int64(stmt, 2, var_id);
    sqlite3_bind_double(stmt, 3, val);
    sqlite3_bind_double(stmt, 4, err);

    busyRetry(stmt);
    sqlite3_reset(stmt);
}

void AnalysisDB::uploadAnaResult(anarun_id_t run_id, anavar_id_t var_id, const string& val) {
    auto stmt = loadStatement("INSERT INTO analysis_xresults(run_id,var_id,val) VALUES (?1,?2,?3)");

    sqlite3_bind_int64(stmt, 1, run_id);
    sqlite3_bind_int64(stmt, 2, var_id);
    bind_string(stmt, 3, val);

    busyRetry(stmt);
    sqlite3_reset(stmt);
}

void AnaResult::display() const {
    printf("%s\t'" TERMFG_GREEN "%s" TERMSGR_RESET "':\t", name.c_str(), descrip.c_str());
    if(xval.size()) printf("%s\t[%s]\n", xval.c_str(), unit.c_str());
    else printf("%12g ~ %-12g\t[%s]\n", val, err, unit.c_str());
}
