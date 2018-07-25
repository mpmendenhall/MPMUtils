/// \file UniqueKeys.hh Unique int key provision
// Michael P. Mendenhall, 2018

#ifndef UNIQUEKEYS_HH
#define UNIQUEKEYS_HH

#include <map>

/// Provide unique enumeration for keys
template<typename K>
class UniqueKeys: protected std::map<K,int> {
public:
    /// get (or create) key saved by name
    size_t getKey(const K& s) {
        auto it = this->find(s);
        if(it != this->end()) return it->second;
        return ((*this)[s] = this->size());
    }
};

#endif
