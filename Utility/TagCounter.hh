/// @file TagCounter.hh templatized counts tally
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

#include <istream>
#include "Stringmap.hh"

/// Templatized counts tally
template<typename T>
class TagCounter: public map<T,double> {
public:
    /// constructor
    TagCounter() { }
    /// constructor from stringmap
    explicit TagCounter(Stringmap m);
    /// add another counter
    void operator+=(const map<T,double>& c);
    /// scale contents
    void operator*=(double s);
    /// multiply all counts
    void scale(double s);
    /// make into Stringmap
    Stringmap toStringmap();
    /// get total counts on all objects
    double total() const;
    /// get count for given item
    double get(const T& itm) const;
};

template<typename T>
void TagCounter<T>::operator+=(const map<T,double>& c) {
    for(auto& kv: c) (*this)[kv.first] += kv.second;
}

template<typename T>
void TagCounter<T>::operator*=(double s) {
    if(s==1) return;
    for(auto& kv: *this) kv.second *= s;
}

template<typename T>
Stringmap TagCounter<T>::toStringmap() {
    Stringmap m;
    for(auto const& kv: *this) {
        std::ostringstream s;
        s << kv.first;
        m.insert(s.str(),to_str(kv.second));
    }
    return m;
}

template<typename T>
double TagCounter<T>::total() const {
    double d = 0;
    for(auto const& kv: *this) d += kv.second;
    return d;
}

template<typename T>
double TagCounter<T>::get(const T& itm) const {
    auto it = this->find(itm);
    return it==this->end()? 0 : it->second;
}

#endif
