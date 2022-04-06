/// \file _AnaIndex.cc

#include "ConfigCollator.hh"

_ConfigCollator* _AnaIndex::makeConfigCollator(const Setting& S) const { return new _ConfigCollator(S); }

SignalSink* _AnaIndex::makeDataSink(const Setting&, const string&) const { return new SignalSink(); }
