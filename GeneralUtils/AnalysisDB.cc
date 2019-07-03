/// \file AnalysisDB.cc
// -- Michael P. Mendenhall, LLNL 2019

#include "AnalysisDB.hh"
#include "CodeVersion.hh"
#include <time.h>
#include <functional> // for std::hash
#include <tuple>

#if BOOST_VERSION < 106900
#include <boost/functional/hash.hpp>
#else
#include <boost/container_hash/hash.hpp>
#endif

AnalysisDB* AnalysisDB::myDB = nullptr;
string AnalysisDB::dbfile = "";

sqlite3_int64 AnalysisDB::code_id() {
    static sqlite3_int64 cid = 0;
    if(!cid) {
        auto descrip = CodeVersion::description();
        cid = std::hash<string>{}(descrip);
        auto stmt = loadStatement("INSERT INTO analysis_code(code_id, code_description) VALUES (?1,?2)");
        sqlite3_bind_int64(stmt, 1, cid);
        sqlite3_bind_text(stmt, 2, descrip.c_str(), -1, SQLITE_STATIC);
        busyRetry(stmt);
        sqlite3_reset(stmt);
    }
    return cid;
}

sqlite3_int64 AnalysisDB::createAnaData(const string& dataname) {
    sqlite3_int64 data_id = std::hash<string>{}(dataname);
    auto stmt = loadStatement("INSERT INTO analysis_runs(data_id, data_name) VALUES (?1,?2)");
    sqlite3_bind_int64(stmt, 1, data_id);
    sqlite3_bind_text(stmt, 2, dataname.c_str(), -1, SQLITE_STATIC);
    busyRetry(stmt);
    sqlite3_reset(stmt);
    return data_id;
}

sqlite3_int64 AnalysisDB::getAnaVar(const string& name, const string& unit, const string& descrip) {
    sqlite3_int64 var_id = std::hash<string>{}(name);
    auto stmt = loadStatement("INSERT OR IGNORE INTO analysis_vars(var_id,name,unit,descrip) VALUES (?1,?2,?3,?4)");
    sqlite3_bind_int64(stmt, 1, var_id);
    sqlite3_bind_text(stmt, 2, name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, unit.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, descrip.c_str(), -1, SQLITE_STATIC);
    busyRetry(stmt);
    sqlite3_reset(stmt);
    return var_id;
}

sqlite3_int64 AnalysisDB::getResultID(sqlite3_int64 data_id, sqlite3_int64 var_id) {
    auto t = time(nullptr);
    sqlite3_int64 result_id = boost::hash_value(std::make_tuple(data_id,var_id,t));

    auto stmt = loadStatement("INSERT INTO analysis_results(result_id, data_id, var_id, anatime) VALUES (?1,?2,?3,?4)");

    sqlite3_bind_int64( stmt, 1, result_id);
    sqlite3_bind_int64( stmt, 2, data_id);
    sqlite3_bind_int64( stmt, 3, var_id);
    sqlite3_bind_double(stmt, 4, t);

    busyRetry(stmt);
    sqlite3_reset(stmt);

    return result_id;
}

void AnalysisDB::uploadAnaResult(sqlite3_int64 result_id, double val, double err) {
    auto stmt = loadStatement("INSERT INTO number_result(result_id, val, err) VALUES (?1,?2,?3)");

    sqlite3_bind_int64(stmt,  1, result_id);
    sqlite3_bind_double(stmt, 2, val);
    sqlite3_bind_double(stmt, 3, err);

    busyRetry(stmt);
    sqlite3_reset(stmt);
}

void AnalysisDB::uploadAnaResult(sqlite3_int64 result_id, const string& val) {
    auto stmt = loadStatement("INSERT INTO text_results(result_id, val) VALUES (?1,?2)");

    sqlite3_bind_int64( stmt, 1, result_id);
    sqlite3_bind_text(  stmt, 2, val.c_str(), -1, SQLITE_STATIC);

    busyRetry(stmt);
    sqlite3_reset(stmt);
}

void AnalysisDB::AnaResult::display() const {
    printf("%s [%s]:\t", name.c_str(), descrip.c_str());
    if(xval.size()) printf("%s [%s]\n", xval.c_str(), unit.c_str());
    else printf("%g ~ %g %s\n", val, err, unit.c_str());
}
