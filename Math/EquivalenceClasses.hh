/// @file "EquivalenceClasses.hh" Helper for assembling equivalence classes
// -- Michael P. Mendenhall, 2019

#ifndef EQUIVALENCECLASSES_HH
#define EQUIVALENCECLASSES_HH

#include "Renumerate.hh"
#include <cassert>
#include <utility>
using std::pair;

/////////////////////////
// <Equivalence Relation> interface:
//
// /// elements being classed together
// typedef <element> elem_t;
//
// /// return if a ~ b; must satisfy a ~ b <==> b ~ a
// bool equiv(const elem_t& a, const elem_t& b) const;
//

///////////////////////////
// <Equivalence Classes> interface:
// <Equivalence Relation> +
//
// /// unique identifier index for an equivalence class
// typedef <identifier> eqidx_t;
//
// /// return equivalence class identifier for element
// eqidx_t classidx(const elem_t& a) const;
//
// /// representative element for class
// const elem_t& representative(eqidx_t i) const;

/// Construct equivalence classes from individual equivalence relations
template<typename T, typename Tidx = size_t>
class EquivalenceClasses: protected map<Tidx, set<T>> {
public:
    /// Element identifier type
    typedef T elem_t;
    /// Equivalence class identifier type
    typedef Tidx eqidx_t;
    /// parent class type
    typedef map<eqidx_t, set<T>> super;

    /// number of equivalence classes
    using super::size;
    using super::begin;
    using super::end;
    /// return equivalence class identifier for element
    eqidx_t classidx(const elem_t& e) const { return egm.at(e); }
    /// check if element already categorized
    bool has(const elem_t& e) const { return egm.count(e); }
    /// equivalence class if element categorized
    pair<bool,eqidx_t> operator()(const elem_t& e) const {
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
    eqidx_t add(const elem_t& a, const elem_t& b) {
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

    /// classify element (potentially in new equivalence class) using supplied operation F(a,b): a==b
    template<typename F>
    eqidx_t classify(const elem_t& a, F equals) {
        for(auto& kv: *this) {
            if(equals(a, *kv.second.begin())) {
                addTo(a, kv.first);
                return kv.first;
            }
        }
        (*this)[ns] = {a};
        egm.emplace(a, ns);
        return ns++;
    }

    /// add to pre-existing equivalence class
    void addTo(const elem_t& e, eqidx_t c) {
        assert(!egm.count(e) || egm[e]==c);
        (*this)[c].insert(e);
        egm[e] = c;
    }

    /// get numbered equivalence class
    const set<T>& getClassNum(eqidx_t n) const {
        static set<T> snull{};
        auto it = this->find(n);
        return it == this->end()? snull : it->second;
    }
    /// get equivalence class of element e; empty if e unclassified
    const set<T>& getClassFor(const elem_t& e) const {
        static set<T> snull{};
        auto it = egm.find(e);
        return it == egm.end()? snull : getClassNum(it->second);
    }
    /// representative element for class
    const elem_t& representative(eqidx_t i) const { return *(this->at(i).begin()); }

    /// apply renumeration
    void renumerate(const renumeration_t<T>& m) {
        for(auto& kv: (super&)*this) kv.second = renumerated(kv.second, m);
        egm = renumerated_key(egm, m);
    }

protected:
    map<elem_t, eqidx_t> egm;   ///< element to equiv group number
    size_t ns = 0;              ///< esets numbering

    /// merge two equivalence classes
    eqidx_t merge(eqidx_t n0, eqidx_t n1) {
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
