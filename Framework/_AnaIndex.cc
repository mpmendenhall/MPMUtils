/// @file _AnaIndex.cc

#include "_AnaIndex.hh"

#include "_ConfigCollator.hh"

_ConfigCollator* _AnaIndex::makeConfigCollator(const ConfigInfo_t& S) const { return new _ConfigCollator(S); }
