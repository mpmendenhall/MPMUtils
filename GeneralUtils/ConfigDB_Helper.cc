/// \file ConfigDB_Helper.cc
// This file was produced under the employ of the United States Government,
// and is consequently in the PUBLIC DOMAIN, free from all provisions of
// US Copyright Law (per USC Title 17, Section 105).
//
// -- Michael P. Mendenhall, 2016

#include "ConfigDB_Helper.hh"

Stringmap ConfigDB_Helper::getConfig(const string& family, const string& name) {
    auto stmt = loadStatement("SELECT rowid FROM config_values WHERE name = ?1 AND family = ?2");
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, family.c_str(), -1, SQLITE_STATIC);

    int rc = busyRetry(stmt);
    sqlite3_int64 n = (rc == SQLITE_ROW)? sqlite3_column_int64(stmt, 0) : -1;
    sqlite3_reset(stmt);

    return getConfig(n);
}

Stringmap ConfigDB_Helper::getConfig(sqlite3_int64 cid) {
    auto stmt = loadStatement("SELECT name,value FROM config_values WHERE csid = ?1");
    sqlite3_bind_int64(stmt, 1, cid);

    Stringmap m;
    while(busyRetry(stmt)== SQLITE_ROW) {
        string k,v;
        get_string(stmt, 0, k);
        get_string(stmt, 1, v);
        m.insert(k,v);
    }
    sqlite3_reset(stmt);
    return m;
}

map<string, Stringmap> ConfigDB_Helper::getConfigs(const string& family) {
    auto stmt = loadStatement("SELECT rowid,name FROM config_set WHERE family = ?1");
    sqlite3_bind_text(stmt, 1, family.c_str(), -1, SQLITE_STATIC);

    vector<sqlite3_int64> ids;
    vector<string> nms;
    while(busyRetry(stmt)== SQLITE_ROW) {
        ids.push_back(sqlite3_column_int64(stmt, 0));
        nms.push_back(string());
        get_string(stmt, 0, nms.back());
    }
    sqlite3_reset(stmt);

    map<string, Stringmap> f;
    for(size_t i=0; i<ids.size(); i++) f.emplace(nms[i],getConfig(ids[i]));
    return f;
}
