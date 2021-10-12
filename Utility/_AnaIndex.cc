/// \file _AnaIndex.cc

#include "ConfigCollator.hh"

_ConfigCollator* _AnaIndex::makeConfigCollator(const Setting& S) const { return new _ConfigCollator(S); }

_DataSink* _AnaIndex::makeDataSink(const Setting&) const { return new _DataSink(); }
