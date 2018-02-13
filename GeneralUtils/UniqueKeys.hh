/// \file UniqueKeys.hh Unique int key provision

#ifndef UNIQUEKEYS_HH
#define UNIQUEKEYS_HH

#include <map>
#include <string>
using std::string;
using std::map;

/// Provide unique (named) enumeration keys
class UniqueKeys {
public:
    /// get (or create) key saved by name
    int getKey(const string& s);
    /// create new anonymous key
    static int getKey();
protected:
    map<string, int> namedKeys; ///< keys with string names
};

#endif
