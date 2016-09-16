/// \file SMFile.hh simple text data format
/*
 * SMFile.hh, part of the MPMUtils package.
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

#ifndef SMFile_HH
#define SMFile_HH

#include "Stringmap.hh"

/// wrapper for multimap<string,Stringmap> with useful functions
class SMFile {
public:
    /// Constructor with input filename
    SMFile(const string& s = "", bool readit = true);

    /// insert key/(string)value pair
    void insert(const string& str, const Stringmap& v);
    /// remove a key
    void erase(const string& str);
    /// retrieve values for key
    vector<Stringmap> retrieve(const string& s) const;
    /// retrieve first value for key
    Stringmap getFirst(const string& str, const Stringmap& dflt = Stringmap()) const;
    /// retrieve all sub-key values
    vector<string> retrieve(const string& k1, const string& k2) const;
    /// retreive sub-key with default
    string getDefault(const string& k1, const string& k2, const string& d) const;
    /// retrieve sub-key as double with default
    double getDefault(const string& k1, const string& k2, double d) const;
    /// retrieve all sub-key values as doubles
    vector<double> retrieveDouble(const string& k1, const string& k2) const;
    /// return number of elements
    unsigned int size() const { return dat.size(); }
    /// transfer all data for given key from other SMFile
    void transfer(const SMFile& Q, const string& k);

    /// set output file location
    void setOutfile(string fnm) { name = fnm; }
    /// commit data to file
    void commit(string outname = "") const;
    /// display to stdout
    void display() const;

    string name;                        ///< output filename
    multimap< string, Stringmap > dat;  ///< key-value multimap
};

#endif
