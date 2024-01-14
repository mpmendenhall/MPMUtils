/// @file Renumerate.hh Utility for swapping numbers/names
// -- Michael P. Mendenhall, 2019

#ifndef RENUMERATE_HH
#define RENUMERATE_HH

#include <set>
using std::set;
#include <map>
using std::map;
#include <vector>
using std::vector;

/// data for re-assigning group element numbers
template<typename T = size_t>
using renumeration_t = map<T,T>;

/// apply renumeration to set
template<typename T>
set<T> renumerated(const set<T>& S, const renumeration_t<T>& m) {
    set<T> SS;
    for(auto& c: S) SS.insert(m.at(c));
    return SS;
}

/// apply renumeration to renumeration
template<typename T>
renumeration_t<T> renum_renum(const renumeration_t<T>& a, const renumeration_t<T>& m) {
    renumeration_t<T> MM;
    for(auto& kv: a) MM.emplace(m.at(kv.first), m.at(kv.second));
    return MM;
}

/// apply renumeration to generic iterable
template<typename T, typename V>
V renumerated(const V& v, const renumeration_t<T>& m) {
    auto u = v;
    for(auto& x: u) x = m.at(x);
    return  u;
}

/// apply renumeration to map key
template<typename T, typename V>
map<T,V> renumerated_key(map<T,V>& M, const renumeration_t<T>& m) {
    map<T,V> MM;
    for(auto& kv: M) MM.emplace(m.at(kv.first), kv.second);
    return MM;
}

/// apply renumeration to map value
template<typename T, typename K>
map<K,T> renumerated_value(const map<K,T>& M, const renumeration_t<T>& m) {
    map<K,T> MM;
    for(auto& kv: M) MM.emplace(kv.first, m.at(kv.second));
    return MM;
}

/// apply renumeration as vector index permutation
template<typename T, typename V>
vector<V> renumerated_permute(const vector<V>& v, const renumeration_t<T>& m) {
    vector<V> vv(v.size());
    size_t i = 0;
    for(auto& c: v) vv.at(m.at(i++)) = c;
    return vv;
}

#endif
