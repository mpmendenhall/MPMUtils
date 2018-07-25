/// \file Stringmap.hh wrapper for multimap<string,string> with useful functions
/*
 * Stringmap.hh, part of the MPMUtils package.
 * Copyright (c) 2018 Michael P. Mendenhall
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

#ifndef STRINGMAP_HH
#define STRINGMAP_HH

#include <map>
#include <vector>
#include "StringManip.hh"

using std::multimap;
using std::vector;
using std::string;

/// wrapper for multimap with extra convenience functions
template<class K, class V>
class xmultimap: public multimap<K,V> {
public:
    /// get first key value (string) or default
    V getDefault(const K& k, const V& d) const {
        auto it = this->find(k);
        return it == this->end()? d : it->second;
    }

    /// retrieve key values
    vector<V> retrieve(const K& k) const {
        vector<V> v;
        for(auto it = this->lower_bound(k); it != this->upper_bound(k); it++) v.push_back(it->second);
        return v;
    }

    /// insert key/(string)value pair
    void insert(const K& k, const V& v) { this->emplace(k,v); }

    /// merge data from another multimap
    void operator+=(const multimap<K,V>& M) { for(auto& kv: M) this->emplace(kv); }
};

/// wrapper for map to string with double retrieval


/// multimap converting between double, string
template<class K>
class xmultimapS: public xmultimap<K,string> {
public:

    using xmultimap<K,string>::insert;
    using xmultimap<K,string>::getDefault;

    /// insert key/(double)value
    void insert(const K& k, double d) {  this->insert(k,to_str(d)); }

    /// get first key value (double) or default
    double getDefault(const K& k, double d) const {
        string s = this->getDefault(k,"");
        if(!s.size()) return d;
        std::stringstream ss(s);
        ss >> d;
        return d;
    }

    /// retrieve key values as doubles
    vector<double> retrieveDouble(const K& k) const {
        vector<double> v;
        double d;
        for(auto const& ss: retrieve(k)) {
            std::stringstream s(ss);
            s >> d;
            v.push_back(d);
        }
        return v;
    }
};

class Stringmap: public xmultimapS<string> {
public:
    /// constructor from a string
    Stringmap(const string& str = "");
    /// convert to string
    string toString() const;
    /// display to stdout
    void display(const string& linepfx = "") const;
};

#endif
