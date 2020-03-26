/// \file AnaGlobals.hh Analysis global variables context definitions
// -- Michael P. Mendenhall, LLNL 2019

#ifndef ANAGLOBALS_HH
#define ANAGLOBALS_HH

#include "ContextMap.hh"

/// Analysis globals, stored in context
namespace AnaGlobals {
    /// example globals struct
    struct s_example: public s_context_singleton<s_example> {
        double x; ///< example global
    };
}

#endif
