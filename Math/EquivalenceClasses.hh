/// \file "EquivalenceClasses.hh" Helper for assembling equivalence classes
// Michael P. Mendenhall, 2019

#ifndef EQUIVALENCECLASSES_HH
#define EQUIVALENCECLASSES_HH

#include <set>
using std::set;
#include <map>
using std::map;
#include <cassert>
#include <utility>
using std::pair;

/// Construct equivalence classes from individual equivalence relations
template<typename T = int>
class EquivalenceClasses: protected map<size_t, set<T>> {
public:
    /// parent class type
    typedef map<size_t, set<T>> super;
    /// Element identifier type
    typedef T elem_t;

    /// number of equivalence classes
    using super::size;
    /// start scan through classes
    auto begin() const { return super::begin(); }
    /// end scan through classes
    auto end() const { return super::end(); }
    /// check if element already categorized
    bool has(const elem_t& e) const { return egm.count(e); }
    /// equivalence class if element categorized
    pair<bool,size_t> operator()(const elem_t& e) const {
        auto it = egm.find(e);
        if(it == egm.end()) return {false,-1};
        return {true,it->second};
    }
    /// check if equivalent
    bool equiv(const elem_t& e0, const elem_t& e1) const {
        if(e0 == e1) return true;
        auto it0 = egm.find(e0);
        if(it0 == egm.end()) return false;
        auto it1 = egm.find(e1);
        if(it1 == egm.end()) return false;
        return it0->first == it1->first;
    }

    /// add equivalency a ~ b; return class number for both
    size_t add(const elem_t& a, const elem_t& b) {
        auto ita = egm.find(a);
        auto itb = egm.find(b);

        if(ita == egm.end() && itb == egm.end()) {
            (*this)[ns] = {a,b};
            egm.emplace(a, ns);
            egm.emplace(b, ns);
            return ns++;
        }

        if(ita == egm.end()) {
            egm.emplace(a, itb->second);
            (*this)[itb->second].insert(a);
            return itb->second;
        }

        if(itb == egm.end()) {
            egm.emplace(b, ita->second);
            (*this)[ita->second].insert(b);
            return ita->second;
        }

        return merge(ita->second, itb->second);
    }

    /// add to pre-existing equivalence class
    void addTo(const elem_t& e, size_t c) {
        assert(!egm.count(e) || egm[e]==c);
        (*this)[c].insert(e);
        egm[e] = c;
    }

    /// get equivalence class of element e
    const set<T>& getClass(const elem_t& e) const {
        static set<T> snull{};
        auto it = this->find(e);
        return it == this->end()? snull : it->second;
    }

protected:
    map<elem_t, size_t> egm;///< element to equiv group number
    size_t ns = 0;          ///< esets numbering

    /// merge two equivalence classes
    size_t merge(size_t n0, size_t n1) {
        if(n1==n0) return n0;
        if(n1 < n0) std::swap(n0,n1);

        auto& s0 = (*this)[n0];
        auto it = this->find(n1);
        for(auto& c: it->second) {
            egm[c] = n0;
            s0.insert(c);
        }
        this->erase(it);
        return n0;
    }
};

#endif
