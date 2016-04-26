/// \file TSQLHelper.hh Convenience wrapper to TSQL database interface
/* 
 * TSQLHelper.hh, part of the MPMUtils package.
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

#ifndef TSQLHELPER_HH
#define TSQLHELPER_HH

#include <TSQLServer.h>
#include <TSQLRow.h>
#include <TSQLResult.h>
#include <vector>
#include <string>
#include <map>
#include "Stringmap.hh"

#define IGNORE_DEAD_DB false

/// convenient wrapper class for DB access
class TSQLHelper {
public:
    
    /// constructor
    TSQLHelper(const std::string& dbName,
              const std::string& dbAddress,
              const std::string& dbUser,
              const std::string& dbPass,
              unsigned int port = 3306,
              unsigned int ntries = 3);
    
    /// destructor
    virtual ~TSQLHelper() { delete res; if(db) db->Close(); }
    
    /// get name of DB in use
    string getDBName() const { return dbName; }
    
    bool isNullResult;  ///< whether field query returned nullptr
    
    char query[9182];   ///< buffer space for SQL query strings
    /// execute a non-info-returning query
    void execute();
    
protected:
    /// use current query string, return first row
    TSQLRow* getFirst();
    /// get field as string (with default for nullptr)
    string fieldAsString(TSQLRow* row, unsigned int fieldnum=0, const std::string& dflt = "");
    /// get field as integer (with default for nullptr)
    int fieldAsInt(TSQLRow* row, unsigned int fieldnum=0, int dflt = 0);
    /// get field as float (with default for nullptr)
    float fieldAsFloat(TSQLRow* row, unsigned int fieldnum=0, float dflt = 0);
    /// display a query result to stdout
    void printResult();
    /// get most recently inserted row ID
    int getInsertID();
    
    /// check if table exists in DB
    bool checkTable(const std::string& tname) { return db && db->HasTable(tname.c_str()); }
    
    /// execute an info-returning query
    void Query();
    
protected:
    TSQLServer* db;             ///< DB server connection
    TSQLResult* res;            ///< result of most recent query
    string dbName;              ///< name of DB in use
};

/// convert a stringmap to "(vars,...) VALUES (vals,...)" for DB insert query
string sm2insert(const Stringmap& m);

#endif

