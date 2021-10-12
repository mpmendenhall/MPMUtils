/// \file _AnaIndex.hh Dynamic type-specific templates lookup
// Michael P. Mendenhall, LLNL 2021

#ifndef _ANAINDEX_HH
#define _ANAINDEX_HH

#include "libconfig_readerr.hh"
#include <stdexcept>
class _ConfigCollator;
class _DataSink;

/// Virtual base for lookup of type-specific analysis chain units
class _AnaIndex {
public:
    virtual _ConfigCollator* makeConfigCollator(const Setting& S) const;
    virtual _DataSink* makeDataSink(const Setting& S) const;
};

#endif
