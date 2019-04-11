/// \file CayleyTable.hh Cayley Table for enumerated (semi)group
// Michael P. Mendenhall, 2019

#ifndef CAYLEYTABLE_HH
#define CAYLEYTABLE_HH

#include "FiniteGroup.hh"
#include <iostream>

/// Construct <Enumerated Semigroup> Cayley Table isomorphism of input <Enumerated Semigroup> G for faster group operations
template<class ESG_t>
class SGCayleyTable {
public:
    /// enumeration type
    typedef typename ESG_t::enum_t enum_t;
    /// Enumerated element type
    typedef enum_t elem_t;

    /// Constructor, from underlying <Enumerated Semigroup>
    SGCayleyTable(const ESG_t& G): iID(G.identity_idx()), order(G.getOrder()) {
        elem_t i = 0;
        for(auto& e1: G) {
            elem_t j = 0;
            for(auto& e2: G) {
                auto k = G.idx(G.apply(e1,e2));
                assert(k < (decltype(k))G.getOrder());
                if(k == iID) inverses[i] = j;
                CT[{i,j++}] = k;
            }
            ++i;
        }
    }

    /// pre-calculated group operator
    elem_t apply(elem_t a, elem_t b) const { auto it = CT.find({a,b}); assert(it != CT.end()); return it->second; }
    /// get group order
    size_t getOrder() const { return order; }
    /// lookup inverse
    enum_t inverse(enum_t i) const { return inverses.at(i); }

    /// return (trivial!) element index
    static constexpr enum_t idx(elem_t i) { return i; }
    /// indexed element
    static constexpr elem_t element(enum_t i) { return i; }
    /// identity element
    const elem_t& identity() const { return iID; }
    /// identity element index
    enum_t identity_idx() const { return iID; }
    /// element iteration start
    auto begin() const { return VRangeIt<elem_t>(order); }
    /// element iteration end
    auto end() const { return VRangeIt<elem_t>(order,order); }

    /// apply renumeration of elements
    void renumerate(const renumeration_t<elem_t>& m) {
        decltype(CT) M;
        for(auto& kv: CT) M.emplace(pair<elem_t,elem_t>(m.at(kv.first.first), m.at(kv.first.second)), m.at(kv.second));
        CT = M;
        auto it = m.find(iID);
        if(it != m.end()) iID = it->second;
        inverses = renum_renum(inverses, m);
    }

protected:
    enum_t iID;                             ///< identity element index
    map<pair<elem_t, elem_t>, elem_t> CT;   ///< Cayley Table ab -> C
    const size_t order;                     ///< number of elements
    map<enum_t, enum_t> inverses;           ///< inverses map
};

/// Cayley Table for <Enumerated Group>
template<class EG_t>
class CayleyTable: public SGCayleyTable<EG_t> {
public:
    typedef typename SGCayleyTable<EG_t>::enum_t enum_t;

    /// Constructor
    CayleyTable(const EG_t& G = {}): SGCayleyTable<EG_t>(G) { }

    /// print table info to stdout
    void display() const {
        using std::cout;
        for(auto& kv: this->inverses) {
            cout << kv.first << " [" << kv.second << "]";
            for(auto& kv2: this->inverses) cout << " " << this->apply(kv.first, kv2.first);
            cout << "\n";
        }
    }
};

#endif
