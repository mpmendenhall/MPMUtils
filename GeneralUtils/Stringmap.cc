/// \file Stringmap.cc
/*
 * Stringmap.cc, part of the MPMUtils package.
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

#include "Stringmap.hh"

Stringmap::Stringmap(const string& str) {
    auto pairs = split(str,"\t");
    for(auto const& s: pairs) {
        auto keyval = split(s,"=");
        if(keyval.size() != 2) continue;
        emplace(strip(keyval[0]),strip(keyval[1]));
    }
}

string Stringmap::toString() const {
    string s;
    for(auto const& kv: *this) s += "\t" + kv.first + " = " + kv.second;
    return s;
}

void Stringmap::display(const string& linepfx) const {
    for(auto const& kv: *this) std::cout << linepfx << kv.first << ": " << kv.second << "\n";
}
