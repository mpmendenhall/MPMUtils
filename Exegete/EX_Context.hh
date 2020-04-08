/// \file EX_Context.hh Call-chain context
/*
 * EX_Context.hh from the Exegete runtime documentation system
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

#include "EX_Note.hh"
#include "EX_Scope.hh"

#include <vector>
using std::vector;
#include <map>
using std::map;
#include <cassert>


namespace EX {

    /// Single-level call tree context
    class Subcontext {
    public:
        /// Constructor
        Subcontext(Scope* s, int d, Subcontext* p);
        /// Destructor
        ~Subcontext() { for(auto& kv: children) delete kv.second; }
        /// Get (create as needed) subcontext for scope
        Subcontext* enterScope(Scope* SS);

        /// print bracket levels to this Subcontext
        void dispbracket(bool edge = false) const;
        /// print chain of scopes through this Subcontext
        void displayScope() const;
        /// make visible, if not already
        void makeVisible();

        Scope* S;                   ///< scope providing this context
        bool visible = false;       ///< whether displayed to output
        int depth;                  ///< depth in call tree
        string dpfx;                ///< display line prefix
        map<int,int> notecounts;    ///< counter for note displays

        Subcontext* parent;                 ///< Subcontext in which this was called
        map<Scope*,Subcontext*> children;   ///< Subcontexts called from this one
    };

    /// Top-level singleton tracking call chain and handling UI
    class Context: protected NoCopy {
    public:
        /// Retrieve context singleton
        static Context& TheContext();
        /// Clear/delete all context information
        static void DeleteContext();

        /// Polymorphic Destructor
        virtual ~Context() { delete current; for(auto& kv: scopes) delete kv.second; }

        /// Get current scope
        Scope& currentScope() { return *current->S; }
        /// Get (or create) idendified scope
        Scope& getScope(Scope::ID id);

        /// Enter (or create) scope identified by ID
        Scope& enterScope(Scope::ID id);
        /// Enter and return new scope if file/function different than before; otherwise, return nullptr
        Scope* requestScope(Scope::ID id);
        /// Exit current scope (optional consistency check)
        void exitScope(Scope::ID id = Scope::ID(nullptr,nullptr,-1));
        /// Trigger enumerated note for current context
        virtual void addNote(int l);

        /// Display current scope information
        void displayScope();

    protected:
        /// Constructor
        Context();
        /// Retrieve context singleton (underlying pointer for cleanup deletion)
        static Context*& _TheContext();

        map<Scope::ID, Scope*> scopes;  ///< index to known scopes
        Subcontext* current;            ///< current position in call chain
    };

}

