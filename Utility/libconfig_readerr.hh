/// \file libconfig_readerr.hh Wrapper for libconfig readFile that prints more verbose error info
// -- Michael P. Mendenhall, 2018

#ifndef LIBCONFIG_READERR_HH
#define LIBCONFIG_READERR_HH

#include <libconfig.h++>
using namespace libconfig;
using std::string;
#include <stdio.h>

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

/// place after try { ... } block for more helpful configuration error printouts
#define SHOW_CFG_ERRORS \
    catch(SettingNotFoundException& e) { \
        printf("Required setting not found: '%s'\n", e.getPath()); throw; \
    } catch(SettingException& e) { \
        printf("Configuration SettingException (wrong type) at '%s'\n", e.getPath()); throw; \
    } catch(ConfigException& e) { \
        printf("Exiting on configuration error.\n"); throw; \
    }

#endif

