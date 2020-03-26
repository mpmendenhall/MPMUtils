/// \file SFINAEFuncs.hh
// -- Michael P. Mendenhall, 2018

#ifndef SFINAEFUNCS_HH
#define SFINAEFUNCS_HH

#include <stdio.h>

/// called when T::display defined
template<typename T>
auto disp_imp(const T& o, int) -> decltype(o.display(), void()) { o.display(); }

/// default called when no T::display defined
template<typename T>
auto disp_imp(const T& o, long) -> decltype(o, void()) { printf("object\n"); }

/// SFINAE black magic to display object if T::display exists
template<typename T>
auto dispObj(const T& o) -> decltype(disp_imp(o,0), void()) { disp_imp(o,0); }

#endif
