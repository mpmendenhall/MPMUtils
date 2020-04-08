/// \file Exegete.hh Central include for Exegete documentation system
/*
 * Exegete.hh from the Exegete runtime documentation system
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

#ifndef EXEGETE_HH
#define EXEGETE_HH

/// define this to enable Exegete functions; undefine to remove them
#ifdef ENABLE_EXEGETE

/// helper functions to glom items together into a token name
#define TOKENCAT(x, y) x ## y
/// extra layer of indirection needed for use in preprocession macros
#define TOKENCAT2(x, y) TOKENCAT(x, y)

#include "EX_Context.hh"
#include "EX_VariableNote.hh"
#include <type_traits>

#ifdef __GNUC__NOPEDONT
#define __myfunc__ __PRETTY_FUNCTION__
#else
#define __myfunc__ __func__
#endif

/// Start a new named scope with a descriptive string
#define _EXSCOPE(S) EX::ScopeGuard TOKENCAT2(_EX_sg_, __LINE__)(EX::Scope::ID(__FILE__, __myfunc__, __LINE__), S);

/// Request start of scope if none previously available (Don't directly call this!)
#define _EXREQSC EX::ScopeRequest TOKENCAT2(_EX_sr_, __LINE__)(EX::Scope::ID(__FILE__, __myfunc__, __LINE__));

/// Simple text comment attached to current scope
#define _EXPLAIN(S) _EXREQSC; EX::Note::makeNote(S, __LINE__);

/// Text comment showing the value of a variable
#define _EXPLAINVAR(S,v) _EXREQSC; EX::VariableNote<decltype(v)>::makeVariableNote(S, __LINE__, #v, v);

/// Text comment on anonymous value
#define _EXPLAINVAL(S,v) _EXREQSC; EX::ValNote<std::remove_reference<decltype(v)>::type>::makeValNote(S, __LINE__, v);

/// Optional, memory cleanup at end of program --- must occur after all annotated scopes have closed
#define _EXEXIT() EX::Context::DeleteContext();

/// Do something only if Exegete is ENABLED
#define _EXONLY(x) x
/// Do something only if Exegete is DISABLED
#define _EXNOPE(x)

#else

#define _EXSCOPE(S)
#define _EXPLAIN(S)
#define _EXPLAINVAR(S,v)
#define _EXPLAINVAL(S,v)
#define _EXEXIT()
#define _EXONLY(x)
#define _EXNOPE(x) x

#endif

#endif

