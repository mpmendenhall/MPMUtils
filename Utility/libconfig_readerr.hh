/// \file libconfig_readerr.hh Wrapper for libconfig readFile that prints more verbose error info
// -- Michael P. Mendenhall, 2018

#ifndef LIBCONFIG_READERR_HH
#define LIBCONFIG_READERR_HH

#include <libconfig.h++>
using namespace libconfig;
using std::string;

/// Default empty configuration
extern const Config NullConfig;
/// Default empty setting
extern const Setting& NullSetting;

/// Read configuration file into config
void readConfigFile(Config& cfg, const string& cfgfile, bool autoconvert = true);

/// Get configuration contents as string
string cfgString(const Config& cfg);

/// Register configuration for lookup by setting; return root setting
const Setting& registerConfig(const Config& cfg);
/// Look up configuration for setting
const Config& lookupConfig(const Setting& S);
/// Look up configuration for setting (nullptr if not registered)
const Config* lookupConfig(const Setting* S);

#endif

