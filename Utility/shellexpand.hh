/// \file shellexpand.h wrapper for <wordexp.h> shell expression wrapper
// -- Michael P. Mendenhall, 2022

#ifndef SHELLEXPAND_HH
#define SHELLEXPAND_HH

#include <string>
using std::string;
#include <vector>
using std::vector;
#include <wordexp.h>

/// shell-expand string into list of strings
vector<string> shellexpand(const string& s);

/// shell expand string into single string, throwing error if wrong multiplicity
string shellexpand_one(const string& s);

/// extract wordexp_t contents to vector<string> and wordfree(&w)
vector<string> to_vs(wordexp_t w);

#endif

