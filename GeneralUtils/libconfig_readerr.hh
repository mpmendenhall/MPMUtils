/// \file libconfig_readerr.hh Wrapper for libconfig readFile that prints more verbose error info
// -- Michael P. Mendenhall, LLNL 2018

#ifndef LIBCONFIG_READERR_HH
#define LIBCONFIG_READERR_HH

#include "libconfig.h++"
using namespace libconfig;
using std::string;

void readConfigFile(Config& cfg, const string& cfgfile, bool autoconvert = true);

#endif

