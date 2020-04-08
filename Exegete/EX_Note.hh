/// \file EX_Note.hh Simple textual annotation note (base class for fancier notes)
/*
 * EX_Note.hh from the Exegete runtime documentation system
 * Copyright (c) 2017 Dr. Michael P. Mendenhall
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

#ifndef EXNOTE_HH
#define EXNOTE_HH

#include <string>
using std::string;

namespace EX {
     /// Annotated commentary on a scope
    class Note {
    public:
        /// Destructor
        virtual ~Note() { }
        /// Get text representation
        virtual string getText() { return S; }

        size_t n = 0;   ///< number of times note has been displayed
        string S;       ///< note contents

        /// add note at line number in current context
        static void makeNote(const string& s, int l);
    protected:
        /// Constructor
        explicit Note(const string& s): S(s) { }
    };

}

#endif
