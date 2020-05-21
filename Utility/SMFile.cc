/// \file SMFile.cc

#include "SMFile.hh"

#include <iostream>
#include <sstream>
#include <fstream>
#include <utility>
#include <stdexcept>
#include "StringManip.hh"

SMFile::SMFile(const string& fname, bool readit) {
    if(!readit || fname=="") return;
    std::ifstream fin(fname.c_str());
    if(!fin.good()) throw std::runtime_error("Unable to read file "+fname);
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
        insert({key, Stringmap(vals)});
    }
    fin.close();
}

vector<Stringmap> SMFile::retrieve(const string& s) const {
    vector<Stringmap> v;
    for(auto it = lower_bound(s); it != upper_bound(s); it++)
        v.push_back(it->second);
    return v;
}

void SMFile::display() const {
    for(auto& kv: *this) {
        std::cout << "--- " << kv.first << " ---:\n";
        kv.second.display();
    }
}

vector<string> SMFile::retrieve(const string& k1, const string& k2) const {
    vector<string> v1;
    for(auto it = lower_bound(k1); it != upper_bound(k1); it++)
        for(auto const& s: it->second.retrieve(k2)) v1.push_back(s);
    return v1;
}

vector<double> SMFile::retrieveDouble(const string& k1, const string& k2) const {
    vector<double> v1;
    for(auto it = lower_bound(k1); it != upper_bound(k1); it++)
        for(auto d: it->second.retrieveDouble(k2)) v1.push_back(d);
    return v1;
}

string SMFile::getDefault(const string& k1, const string& k2, const string& d) const {
    for(auto it = lower_bound(k1); it != upper_bound(k1); it++) {
        vector<string> v2 = it->second.retrieve(k2);
        if(v2.size()) return v2[0];
    }
    return d;
}

double SMFile::getDefault(const string& k1, const string& k2, double d) const {
    for(auto it = lower_bound(k1); it != upper_bound(k1); it++) {
        vector<double> v2 = it->second.retrieveDouble(k2);
        if(v2.size()) return v2[0];
    }
    return d;
}

Stringmap SMFile::getFirst(const string& s, const Stringmap& dflt) const {
    auto it = find(s);
    if(it == end()) return dflt;
    return it->second;
}
