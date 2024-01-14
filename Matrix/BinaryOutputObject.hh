/// @file BinaryOutputObject.hh base utility class for tag-string-delimited binary IO
/*
 * BinaryOutputObject.hh, part of the MPMUtils package.
 * Copyright (c) 2007-2014 Michael P. Mendenhall
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

#ifndef BINARYOUTPUTOBJECT_HH
/// Make sure this header is only loaded once
#define BINARYOUTPUTOBJECT_HH

#include <iostream>
#include <stdio.h>
#include <string>
#include <stdexcept>

using std::string;

/// virtual base class with functions for objects that write themselves to file
class BinaryOutputObject {
public:
    /// constructor
    BinaryOutputObject() {}
    /// destructor
    virtual ~BinaryOutputObject() {}

    /// write string to file
    static void writeString(const string& s, std::ostream& o) {
        o.write(s.c_str(), s.size());
        o.flush();
    }
    /// check whether file matches string
    static bool checkString(const string& s, std::istream& i, bool throwIfMismatch = true) {
        string s2(s.size(),'x');
        i.read(&s2[0],s2.size());
        if(throwIfMismatch && s2!=s) throw std::runtime_error("Mismatched_file_string; expected '" + s + "'; got '" + s2 + "'");
        return s2==s;
    }
};

#endif
