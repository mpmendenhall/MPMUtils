/// @file GlobalArgs.hh Utilities for command line argument globals
// Michael P. Mendenhall, LLNL 2021

#ifndef GLOBALARGS_HH
#define GLOBALARGS_HH

#include <string>
using std::string;
#include <vector>
using std::vector;
#include <map>
#include <set>
#include <stdlib.h> // for atof, atoi

/// string-tagged arguments context singleton
std::map<string, vector<string>>& GlobalArgs();
/// set of arguments that have been queried
std::set<string>& QueriedArgs();

/// load command-line arguments into GlobalArgs() list
void loadGlobalArgs(int argc, char** argv);

/// set value for argument if not already specified
void setDefaultGlobalArg(const string& argname, const string& argval);

/// get number of times argument was specified
size_t numGlobalArg(const string& argname);
/// check whether "+argname" given, with status message printout
bool wasArgGiven(const string& argname, const string& help = "");

/// get required single-valued command line argument or throw error
string requiredGlobalArg(const string& argname, const string& help = "");
/// get required one-or-more-valued command-line argument
const vector<string>& requiredGlobalMulti(const string& argname, const string& help = "", size_t nmin = 1);
/// get required single-valued command line argument or throw error
inline void requiredGlobalArg(const string& argname, double& v, const string& help = "") { v = atof(requiredGlobalArg(argname,help).c_str()); }
/// get required single-valued command line argument or throw error
inline void requiredGlobalArg(const string& argname, int& v, const string& help = "") { v = atoi(requiredGlobalArg(argname,help).c_str()); }

/// pop one of multi-valued global arg (throw if none)
string popGlobalArg(const string& argname);

/// get optional string argument or default
string optionalGlobalDefault(const string& argname, const string& dflt, const string& help = "");
/// get optional argument, or leave with default
bool optionalGlobalArg(const string& argname, string& v, const string& help = "");
/// update value with optional global floating-point argument
bool optionalGlobalArg(const string& argname, double& v, const string& help = "");
/// update value with optional global floating-point argument
bool optionalGlobalArg(const string& argname, int& v, const string& help = "");
/// update value with optional global bool argument (accept '+' form for true)
bool optionalGlobalArg(const string& argname, bool& v, const string& help = "");

/// intepret string as bool: nonzero number or start char in 'TtYy' -> true
bool string_to_bool(const string& s);

/// debugging printout of global args
void displayGlobalArgs();
/// printout unused global arg warnings; return number of unused args found
int checkUnusedArgs();

#endif
