/// @file ConfigCollator.hh Configturation-buildable Collator object
// Michael P. Mendenhall, LLNL 2021

#ifndef CONFIGCOLLATOR_HH
#define CONFIGCOLLATOR_HH

#include "_ConfigCollator.hh"
#include "Collator.hh"

/// Configturation-buildable Collator object
template<class T>
class ConfigCollator: public _ConfigCollator, public Collator<T> {
public:
    /// Constructor
    explicit ConfigCollator(const Setting& S): _ConfigCollator(S) {
        if(S.exists("next")) createOutput(S["next"]);
    }
};

/// Registration in AnaIndex
template<typename T>
_ConfigCollator* AnaIndex<T,typename T::ordering_t>::makeConfigCollator(const Setting& S) const { return new ConfigCollator<T>(S); }

#endif
