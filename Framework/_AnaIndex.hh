/// @file _AnaIndex.hh Dynamic type-specific templates lookup
// Michael P. Mendenhall, LLNL 2021

#ifndef _ANAINDEX_HH
#define _ANAINDEX_HH

#include "ConfigFactory.hh"
#include <stdexcept>
class _ConfigCollator;

/// Virtual base for lookup of type-specific analysis chain units
class _AnaIndex {
public:
    virtual _ConfigCollator* makeConfigCollator(const ConfigInfo_t& S) const;
};

#endif
