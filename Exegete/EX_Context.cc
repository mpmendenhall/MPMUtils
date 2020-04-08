/// \file EX_Context.cc
/*
 * EX_Context.cc from the Exegete runtime documentation system
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

#include "EX_Context.hh"
using namespace EX;
#include <cmath>
#include <string.h>
#include "TermColor.hh"

Subcontext::Subcontext(Scope* s, int d, Subcontext* p): S(s), depth(d), parent(p) {
    if(depth % 2) dpfx = TERMFG_YELLOW "|" TERMSGR_RESET;
    else dpfx = TERMFG_RED "|" TERMSGR_RESET;
}

Subcontext* Subcontext::enterScope(Scope* SS) {
    auto it = children.find(SS);
    if(it != children.end()) return it->second;
    return (children[SS] = new Subcontext(SS, depth+1, this));
}

void Subcontext::dispbracket(bool edge) const {
    if(parent) {
        parent->dispbracket();
        if(edge) {
            if(depth % 2) printf(TERMFG_YELLOW "+--" TERMSGR_RESET);
            else printf(TERMFG_RED "+--" TERMSGR_RESET);
        } else printf("%s", dpfx.c_str());
    }
}

void Subcontext::displayScope() const {
    if(!parent) return;
    if(parent->parent) { parent->displayScope(); printf(" > "); }
    else printf(TERMFG_CYAN);
    printf("%s", S->getName().c_str());
}

void Subcontext::makeVisible() {
    if(visible || !S) return;
    visible = true;
    dispbracket(true);
    printf(" ");
    displayScope();
    if(S->descrip.size()) printf(TERMFG_BLUE " '%s'", S->descrip.c_str());
    printf(TERMSGR_RESET "\n");
}

//-----------------------

Context::Context() {
     current = new Subcontext(&getScope(Scope::ID("cosmos","being",0)), -1, nullptr);
     current->visible = true;
}

Scope& Context::getScope(Scope::ID id) {
    auto it = scopes.find(id);
    if(it != scopes.end()) return *it->second;
    return *(scopes[id] = new Scope(id));
}

Scope& Context::enterScope(Scope::ID id) {
    auto& S = getScope(id);
    current = current->enterScope(&S);
    return S;
}

Scope* Context::requestScope(Scope::ID id) {
    auto S = current->S;
    if(!S || (!strcmp(get<1>(S->id), get<1>(id)) && !strcmp(get<0>(S->id), get<0>(id)))) return nullptr;
    return &enterScope(id);
}

void Context::exitScope(Scope::ID id) {
    if(get<0>(id)) assert(current->S && id == current->S->id);
    if(current->visible) { current->dispbracket(true); printf("\n"); }
    current->visible = false;
    current = current->parent;
    assert(current);
}

Context*& Context::_TheContext() {
    static Context* C = nullptr;
    return C;
}

Context& Context::TheContext() {
    auto& C = _TheContext();
    if(!C) C = new Context();
    return *C;
}

void Context::DeleteContext() {
    auto& C = _TheContext();
    assert(!C->current->parent);
    delete C;
    C = nullptr;
}

inline bool doDisplay(int i) {
    int ii = pow(10, (int)log10(i));
    return !(i%ii) && i/ii <= 3;
}

void Context::addNote(int l) {
    auto n = current->S->getNote(l);
    assert(n);
    int nrpt = ++current->notecounts[l];
    if(doDisplay(nrpt)) {
        current->makeVisible();
        current->dispbracket();
        printf(TERMFG_BLUE " [%s:%i", get<0>(current->S->id), l);
        if(nrpt > 1) printf(" #%i", nrpt);
        printf("] " TERMFG_GREEN "%s\n" TERMSGR_RESET, n->getText().c_str());
    }
}

