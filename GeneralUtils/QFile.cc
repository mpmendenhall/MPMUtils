/* 
 * QFile.cc, part of the MPMUtils package.
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

#include "QFile.hh"

#include <iostream>
#include <sstream>
#include <fstream>
#include <utility>
#include "StringManip.hh"
#include "PathUtils.hh"
#include "SMExcept.hh"

QFile::QFile(const string& fname, bool readit) {
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

void QFile::insert(const string& s, const Stringmap& v) {
    dat.insert(std::make_pair(s,v));
}

void QFile::erase(const string& s) { dat.erase(s); }

vector<Stringmap> QFile::retrieve(const string& s) const {
    vector<Stringmap> v;
    for(multimap<string,Stringmap>::const_iterator it = dat.lower_bound(s); it != dat.upper_bound(s); it++)
        v.push_back(it->second);
    return v;
}

void QFile::transfer(const QFile& Q, const string& k) {
    vector<Stringmap> v = Q.retrieve(k);
    for(vector<Stringmap>::iterator it = v.begin(); it != v.end(); it++)
        insert(k,*it);
}

void QFile::display() const {
    for(multimap<string, Stringmap>::const_iterator it = dat.begin(); it != dat.end(); it++) {
        std::cout << "--- " << it->first << " ---:\n";
        it->second.display();
    }
}

void QFile::commit(string outname) const {
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
    for(multimap<string, Stringmap>::const_iterator it = dat.begin(); it != dat.end(); it++)
        fout << it->first << ":\t" << it->second.toString() << "\n";
    fout.close();
}

vector<string> QFile::retrieve(const string& k1, const string& k2) const {
    vector<string> v1;
    for(multimap<string,Stringmap>::const_iterator it = dat.lower_bound(k1); it != dat.upper_bound(k1); it++) {
        vector<string> v2 = it->second.retrieve(k2);
        for(vector<string>::const_iterator it2 = v2.begin(); it2 != v2.end(); it2++)
            v1.push_back(*it2);
    }
    return v1;
}

vector<double> QFile::retrieveDouble(const string& k1, const string& k2) const {
    vector<double> v1;
    for(multimap<string,Stringmap>::const_iterator it = dat.lower_bound(k1); it != dat.upper_bound(k1); it++) {
        vector<double> v2 = it->second.retrieveDouble(k2);
        for(vector<double>::const_iterator it2 = v2.begin(); it2 != v2.end(); it2++)
            v1.push_back(*it2);
    }
    return v1;
}

string QFile::getDefault(const string& k1, const string& k2, const string& d) const {
    for(multimap<string,Stringmap>::const_iterator it = dat.lower_bound(k1); it != dat.upper_bound(k1); it++) {
        vector<string> v2 = it->second.retrieve(k2);
        if(v2.size())
            return v2[0];
    }
    return d;
}

double QFile::getDefault(const string& k1, const string& k2, double d) const {
    for(multimap<string,Stringmap>::const_iterator it = dat.lower_bound(k1); it != dat.upper_bound(k1); it++) {
        vector<double> v2 = it->second.retrieveDouble(k2);
        if(v2.size())
            return v2[0];
    }
    return d;
}

Stringmap QFile::getFirst(const string& s, const Stringmap& dflt) const {
    multimap<string,Stringmap>::const_iterator it = dat.find(s);
    if(it == dat.end())
        return dflt;
    return it->second;
}
