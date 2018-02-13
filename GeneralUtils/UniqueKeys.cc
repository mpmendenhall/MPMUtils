/// \file UniqueKeys.cc

#include "UniqueKeys.hh"

int UniqueKeys::getKey() {
    static int k=0;
    return k++;
}

int UniqueKeys::getKey(const string& s) {
    auto it = namedKeys.find(s);
    if(it != namedKeys.end()) return it->second;
    return (namedKeys[s] = getKey());
}
