/// @file AnaIndex.hh Dynamic type-specific templates lookup
// Michael P. Mendenhall, LLNL 2021

#ifndef ANAINDEX_HH
#define ANAINDEX_HH

#include "_AnaIndex.hh"

/// Type-specific functions index,
template<typename T, typename = void>
class AnaIndex: virtual public _AnaIndex { };

/// Specialize when ordering available
template<typename T>
class AnaIndex<T, typename T::ordering_t>: virtual public _AnaIndex {
public:
    /// make type-appropriate ConfigCollator
    _ConfigCollator* makeConfigCollator(const ConfigInfo_t& S) const override;
};

#include "ConfigCollator.hh"

#endif
