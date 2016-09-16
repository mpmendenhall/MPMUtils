/// \file TagCounter.hh templatized counts tally
/*
 * TagCounter.hh, part of the MPMUtils package.
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

#ifndef TAGCOUNTER_HH
#define TAGCOUNTER_HH

#include <map>
#include <iostream>
#include <istream>
#include <sstream>
#include "Stringmap.hh"
#include "StringManip.hh"

using std::map;

/// Templatized counts tally
template<typename T>
class TagCounter {
public:
    /// constructor
    TagCounter() { }
    /// constructor from stringmap
    TagCounter(Stringmap m);
    /// destructor
    ~TagCounter() {}
    /// add counts
    void add(const T& itm, double c);
    /// add another counter
    void operator+=(const TagCounter<T>& c);
    /// multiply all counts
    void scale(double s);
    /// make into Stringmap
    Stringmap toStringmap();
    /// get number of counted items
    unsigned int nTags() const { return counts.size(); }
    /// get total counts on all objects
    double total() const;
    /// get count for given item
    double operator[](const T& itm) const;

    map<T,double> counts;  ///< counts per object
};

template<typename T>
void TagCounter<T>::add(const T& itm, double c) {
    counts[itm] += c;
}

template<typename T>
void TagCounter<T>::operator+=(const TagCounter<T>& c) {
    for(auto const& kv: c.counts)
        add(kv.first,kv.second);
}

template<typename T>
void TagCounter<T>::scale(double s) {
    if(s==1) return;
    for(auto& kv: counts)
        kv.second *= s;
}

template<typename T>
Stringmap TagCounter<T>::toStringmap() {
    Stringmap m;
    for(auto const& kv: counts) {
        std::ostringstream s;
        s << kv.first;
        m.insert(s.str(),to_str(kv.second));
    }
    return m;
}

template<typename T>
double TagCounter<T>::total() const {
    double d = 0;
    for(auto const& kv: counts) d += kv.second;
    return d;
}

template<typename T>
double TagCounter<T>::operator[](const T& itm) const {
    typename map<T,double>::const_iterator it = counts.find(itm);
    if(it==counts.end()) return 0;
    return it->second;
}

#endif
