/// \file Stringmap.hh wrapper for multimap<string,string> with useful functions
/*
 * Stringmap.hh, part of the MPMUtils package.
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

#ifndef STRINGMAP_HH
#define STRINGMAP_HH

#include <map>
#include <vector>
#include <string>

using std::map;
using std::multimap;
using std::vector;
using std::string;

/// wrapper for multimap<string,string> with useful functions
class Stringmap {
public:
    /// constructor from a string
    Stringmap(const string& str = "");
    /// destructor
    virtual ~Stringmap() {}


    /// insert key/(string)value pair
    void insert(const string& str, const string& v);
    /// insert key/(double)value
    void insert(const string& str, double d);
    /// retrieve key values
    vector<string> retrieve(const string& str) const;
    /// get first key value (string) or default
    string getDefault(const string& str, const string& d) const;
    /// return number of elements
    unsigned int size() const { return dat.size(); }
    /// return count of entries with key
    unsigned int count(const string& str) const { return dat.count(str); }
    /// serialize to a string
    string toString() const;

    /// get first key value (double) or default
    double getDefault(const string& str, double d) const;
    /// retrieve key values as doubles
    vector<double> retrieveDouble(const string& str) const;
    /// remove a key
    void erase(const string& str);

    /// display to screen
    void display(string linepfx = "") const;

    /// merge data from another stringmap
    void operator+=(const Stringmap& S) { S.mergeInto(*this); }

    multimap<string, string> dat;       ///< key-value multimap

protected:

    /// merge data into another stringmap
    void mergeInto(Stringmap& S) const;
};

#endif
