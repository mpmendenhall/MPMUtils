/// \file AnaIndex.hh Dynamic type-specific templates lookup
// Michael P. Mendenhall, LLNL 2021

#ifndef ANAINDEX_HH
#define ANAINDEX_HH

#include "_AnaIndex.hh"

/// Type-specific functions index
template<typename T>
class AnaIndex: virtual public _AnaIndex {
public:
    /// make type-appropriate ConfigCollator
    _ConfigCollator* makeConfigCollator(const Setting& S) const override;
    /// make type-appropriate DataSink
    SignalSink* makeDataSink(const Setting& S, const string& dfltclass = "") const override;
};

#endif
