/// @file TSQLHelper.cc
/*
 * TSQLHelper.cc, part of the MPMUtils package.
 * Copyright (c) 2007-2014 Michael P. Mendenhall
 *
 * This code uses the LAPACKE C interface to LAPACK;
 * see http://www.netlib.org/lapack/lapacke.html
 * and the GSL interface to CBLAS, https://www.gnu.org/software/gsl/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "TSQLHelper.hh"
#include <stdlib.h>
#include <unistd.h>
#include <stdexcept>

TSQLHelper::TSQLHelper(const std::string& dbnm,
                     const std::string& dbAddress,
                     const std::string& dbUser,
                     const std::string& dbPass,
                     unsigned int port,
                     unsigned int ntries): db(nullptr), res(nullptr), dbName(dbnm) {

    string dbAddressFull = "mysql://"+dbAddress+":"+to_str(port)+"/"+dbnm;
    while(!db) {
        ntries--;
        db = TSQLServer::Connect(dbAddressFull.c_str(),dbUser.c_str(),dbPass.c_str());
        if(!db) {
            if(!ntries || IGNORE_DEAD_DB)
                break;
            printf("** DB Connection %s@%s failed... retrying...\n",dbUser.c_str(),dbAddressFull.c_str());
            sleep(2);
        } else {
            printf("Connected to DB server: %s\n", db->ServerInfo());
            return;
        }
    }
    throw std::runtime_error("Failed to connect to DB " + dbUser +"@" + dbAddressFull);
}

void TSQLHelper::execute(const string& query) {
    delete res;
    res = nullptr;
    if(!db->Exec(query.c_str())) throw std::runtime_error("DB Exec Failed: " + query);
}

void TSQLHelper::Query(const string& query) {
    if(!db) {
        res = nullptr;
    } else {
        delete res;
        res = db->Query(query.c_str());
        if(db->GetErrorCode()) throw std::runtime_error("DB Query Failed: " + query);
    }
}

string TSQLHelper::fieldAsString(TSQLRow* row, unsigned int fieldnum, const std::string& dflt) {
    const char* s = row->GetField(fieldnum);
    isNullResult = !s;
    if(isNullResult) return dflt;
    return std::string(s);
}

int TSQLHelper::fieldAsInt(TSQLRow* row, unsigned int fieldnum, int dflt) {
    string s = fieldAsString(row,fieldnum);
    if(isNullResult)
        return dflt;
    return atoi(s.c_str());
}

float TSQLHelper::fieldAsFloat(TSQLRow* row, unsigned int fieldnum, float dflt) {
    string s = fieldAsString(row,fieldnum);
    if(isNullResult)
        return dflt;
    return atof(s.c_str());
}

int TSQLHelper::getInsertID() {
    TSQLRow* r = getFirst("SELECT LAST_INSERT_ID()");
    if(!r) throw std::runtime_error("LAST_INSERT_ID query failed");
    int rid = fieldAsInt(r,0);
    delete r;
    if(!rid) throw std::runtime_error("Insertion failed");
    return rid;
}

void TSQLHelper::printResult() {
    TSQLRow* row;
    while( (row = res->Next()) ) {
        printf("----------------\n");
        for(int i=0; i<res->GetFieldCount(); i++)
            printf("%s:\t%s\n",res->GetFieldName(i),fieldAsString(row,i,"nullptr").c_str());
        delete row;
    }
}

TSQLRow* TSQLHelper::getFirst(const string& query) {
    Query(query);
    if(!res) return nullptr;
    return res->Next();
}

string sm2insert(const Stringmap& m) {
    string svars = "(";
    string svals = "VALUES (";
    for(auto it = m.begin(); it != m.end(); ++it) {
        if(it != m.begin()) {
            svars += ",";
            svals += ",";
        }
        svars += std::string("`")+it->first+"`";
        svals += it->second;
    }
    return svars+") "+svals+")";
}

