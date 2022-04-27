/// \file GlobalArgs.hh Utilities for command line argument globals
// Michael P. Mendenhall, LLNL 2021

#ifndef GLOBALARGS_HH
#define GLOBALARGS_HH

#include <string>
using std::string;
#include <vector>
#include <map>
#include <stdlib.h> // for atof, atoi

/// string-tagged arguments context singleton
std::map<string, std::vector<string>>& GlobalArgs();

/// load command-line arguments into GlobalArgs() list
void loadGlobalArgs(int argc, char** argv);

/// get number of times argument was specified
size_t numGlobalArg(const string& argname);
/// check whether "+argname" given, with status message printout
bool wasArgGiven(const string& argname, const string& help = "");

/// get required single-valued command line argument or throw error
const string& requiredGlobalArg(const string& argname, const string& help = "");
/// get required single-valued command line argument or throw error
inline void requiredGlobalArg(const string& argname, double& v, const string& help = "") { v = atof(requiredGlobalArg(argname,help).c_str()); }
/// get required single-valued command line argument or throw error
inline void requiredGlobalArg(const string& argname, int& v, const string& help = "") { v = atoi(requiredGlobalArg(argname,help).c_str()); }

/// pop one of multi-valued global arg (throw if none)
string popGlobalArg(const string& argname);

/// get optional argument, or leave with default
bool optionalGlobalArg(const string& argname, string& v, const string& help = "");
/// update value with optional global floating-point argument
bool optionalGlobalArg(const string& argname, double& v, const string& help = "");
/// update value with optional global floating-point argument
bool optionalGlobalArg(const string& argname, int& v, const string& help = "");
/// update value with optional global boolean argument
bool optionalGlobalArg(const string& argname, bool& v, const string& help = "");

/// debugging printout of global args
void displayGlobalArgs();

#endif
