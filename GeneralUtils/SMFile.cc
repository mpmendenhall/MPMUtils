/// \file SMFile.cc
/*
 * SMFile.cc, part of the MPMUtils package.
 * Copyright (c) 2014 Michael P. Mendenhall
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

#include "SMFile.hh"

#include <iostream>
#include <sstream>
#include <fstream>
#include <utility>
#include "StringManip.hh"
#include "PathUtils.hh"
#include "SMExcept.hh"

SMFile::SMFile(const string& fname, bool readit) {
    name = fname;
    if(!readit || name=="")
        return;
    if(!fileExists(fname)) {
        SMExcept e("fileUnreadable");
        e.insert("filename",fname);
        throw(e);
    }
    std::ifstream fin(fname.c_str());
    string s;
    while (fin.good()) {
        std::getline(fin,s);
        s = strip(s);
        size_t n = s.find(':');
        if(n==string::npos || s[0]=='#') continue;
        string key = s.substr(0,n);
        string vals = s.substr(n+1);
        vals=strip(vals);
        while(vals.size() && vals[vals.size()-1]=='\\') {
            vals.erase(vals.size()-1);
            std::getline(fin,s);
            s = strip(s);
            vals += '\t';
        vals += s;
        }
        insert(key,Stringmap(vals));
    }
    fin.close();
}

void SMFile::insert(const string& s, const Stringmap& v) {
    dat.emplace(s,v);
}

void SMFile::erase(const string& s) { dat.erase(s); }

vector<Stringmap> SMFile::retrieve(const string& s) const {
    vector<Stringmap> v;
    for(auto it = dat.lower_bound(s); it != dat.upper_bound(s); it++)
        v.push_back(it->second);
    return v;
}

void SMFile::transfer(const SMFile& Q, const string& k) {
    for(auto const& sm: Q.retrieve(k)) insert(k,sm);
}

void SMFile::display() const {
    for(auto const& kv: dat) {
        std::cout << "--- " << kv.first << " ---:\n";
        kv.second.display();
    }
}

void SMFile::commit(string outname) const {
    if(outname=="")
        outname = name;
    makePath(outname,true);
    std::ofstream fout(outname.c_str());
    if(!fout.good()) {
        SMExcept e("fileUnwriteable");
        e.insert("filename",outname);
        throw(e);
    }
    printf("Writing File '%s'.\n",outname.c_str());
    for(auto const& kv: dat) fout << kv.first << ":\t" << kv.second.toString() << "\n";
    fout.close();
}

vector<string> SMFile::retrieve(const string& k1, const string& k2) const {
    vector<string> v1;
    for(auto it = dat.lower_bound(k1); it != dat.upper_bound(k1); it++)
        for(auto const& s: it->second.retrieve(k2)) v1.push_back(s);
    return v1;
}

vector<double> SMFile::retrieveDouble(const string& k1, const string& k2) const {
    vector<double> v1;
    for(auto it = dat.lower_bound(k1); it != dat.upper_bound(k1); it++)
        for(auto d: it->second.retrieveDouble(k2)) v1.push_back(d);
    return v1;
}

string SMFile::getDefault(const string& k1, const string& k2, const string& d) const {
    for(auto it = dat.lower_bound(k1); it != dat.upper_bound(k1); it++) {
        vector<string> v2 = it->second.retrieve(k2);
        if(v2.size()) return v2[0];
    }
    return d;
}

double SMFile::getDefault(const string& k1, const string& k2, double d) const {
    for(auto it = dat.lower_bound(k1); it != dat.upper_bound(k1); it++) {
        vector<double> v2 = it->second.retrieveDouble(k2);
        if(v2.size()) return v2[0];
    }
    return d;
}

Stringmap SMFile::getFirst(const string& s, const Stringmap& dflt) const {
    auto it = dat.find(s);
    if(it == dat.end()) return dflt;
    return it->second;
}
