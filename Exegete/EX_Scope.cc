/// \file EX_Scope.cc
/*
 * EX_Scope.cc from the Exegete runtime documentation system
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

#include "EX_Scope.hh"
#include "EX_Context.hh"
using namespace EX;

Note*& Scope::getNote(int l) {
    auto it = notes.find(l);
    if(it != notes.end()) return it->second; 
    return (notes[l] = nullptr);
}

ScopeGuard::ScopeGuard(Scope::ID i, const string& descrip): S(Context::TheContext().enterScope(i)) {
    S.descrip = descrip;
}

ScopeGuard::~ScopeGuard() {
    Context::TheContext().exitScope(S.id);
}

//--------------------------

ScopeRequest::ScopeRequest(Scope::ID i): S(Context::TheContext().requestScope(i)) { }

ScopeRequest::~ScopeRequest() {
    if(S) Context::TheContext().exitScope(S->id);
}
