/// \file GetEnv.hh Helper to get environment variable

#ifndef GETENV_HH
#define GETENV_HH

#include <string>
using std::string;

/// project-specific prefix for environment vars
extern const string PROJ_ENV_PFX;

/// update `val` with env[name] if existing; return whether found; optional throw error if missing
bool getEnv(const string& name, string& val, bool fail_if_missing = false);

/// get environment variable ("" if not found); optional throw error if missing
string getEnv(const string& name, bool fail_if_missing = false);

#endif
