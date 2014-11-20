/* 
 * TagCounter.cc, part of the MPMUtils package.
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

#include "TagCounter.hh"
#include "StringManip.hh"

#include <stdlib.h>
#include <cassert>
#include <utility>

template<>
TagCounter<int>::TagCounter(Stringmap m) {
    for(std::multimap< std::string, string >::iterator it = m.dat.begin(); it != m.dat.end(); it++)
        add(atoi(it->first.c_str()),atof(it->second.c_str()));
}

template<>
TagCounter<unsigned int>::TagCounter(Stringmap m) {
    for(std::multimap< std::string, string >::iterator it = m.dat.begin(); it != m.dat.end(); it++)
        add(atoi(it->first.c_str()),atof(it->second.c_str()));
}

template<>
TagCounter<string>::TagCounter(Stringmap m) {
    for(std::multimap< std::string, string >::iterator it = m.dat.begin(); it != m.dat.end(); it++)
        add(it->first,atof(it->second.c_str()));
}

template<>
TagCounter< std::pair<unsigned int, unsigned int> >::TagCounter(Stringmap m) {
    for(std::multimap< std::string, string >::iterator it = m.dat.begin(); it != m.dat.end(); it++) {
        vector<int> v = sToInts(it->first, "/");
        assert(v.size()==2);
        if(v.size()==2)
            add(std::pair<unsigned int, unsigned int>(v[0],v[1]),atof(it->second.c_str()));
    }
}
