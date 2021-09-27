/// \file "FiniteGroup.hh" Base classes for finite groups
// -- Michael P. Mendenhall, 2019

/*
 * Simple group G: only normal subgroups are {e} and G
 * Non-simple: can be divided G/S, where S is a subgroup
 *
 * Center of a group Z(G) = elements that commute with all elements in G
 *      - is a normal subgroup.
 *
 * Centralizer of g in G: set of elements that commute with g
 *
 * Conjugacy classes: equivalence relation, a ~ b if there exists g s.t. g a g^-1 = b
 * a ~ b => a^k ~ b^k
 * "Class number of G" = number of distinct classes
 * Conjugate elements have same order.
 * Every element a in center Z(G) is its own class {a}
 *  => identity in any G, every element in Abelian G is its own class
 *
 * Direct products GxH: G, H are normal in GxH
 * Conjugacy classes, centers are Cartesian products of those for G,H
 * if A normal in G, B normal in H, then AxB normal in GxH with (GxH)/(AxB) isomorphic to (G/A)x(H/B)
 *
 * Solvable group: can be constructed from Abelian groups with extensions
 * Every group of odd order is solvable => "every finite simple group has even order unless it is cyclic of prime order."
 *
 */

/////////////////////////
// <Semigroup> interface: defines closure under operator
//
// typedef <element> elem_t;                // type for Semigroup element
// elem_t apply(elem_t a, elem_t b) const;  // apply semigroup operator to get c = ab

////////////////////////////////////
// <Enumerated Semigroup> interface:
// <Semigroup> plus:
//
// typedef size_t enum_t;               // enumeration type
//
// size_t getOrder() const;             // number of elements
// <iterator> begin() const;            // for iterating elements
// <iterator> end() const;              // for iterating elements
// enum_t idx(elem_t) const;            // enumeration index for element
// elem_t element(enum_t i) const;      // get enumerated element

///////////////////////////////////
// <[Enumerated] Monoid> interface:
// <[Enumerated] Semigroup> plus:
//
// elem_t identity() const; // get identity element

///////////////////////////////////
// <[Enumerated] Group> interface:
// <[Enumerated] Monoid> plus:
//
// elem_t inverse(elem_t a) const;  // return a^-1

#ifndef FINITEGROUP_HH
#define FINITEGROUP_HH

#include "RangeIt.hh"
#include "Renumerate.hh"
#include <utility>
using std::pair;
#include <iostream>
#include <algorithm>

//////////////////////////////////
// basic (semi)group constructions
//////////////////////////////////

/// <Semigroup> wrapper to use '*' as operation
template<class T>
class MultiplySG {
public:
    /// element representation
    typedef T elem_t;
    /// apply a*b
    static constexpr elem_t apply(const elem_t& a, const elem_t& b) { return a*b; }
};

/// <Group> wrapper to use '*' as operation
template<class T>
class MultiplyG: public MultiplySG<T> {
public:
    /// inverse
    static constexpr T inverse(const T& e) { return e.inverse(); }
};

/// chain semigroup operations a, {b,c,...} -> ...*c*b*a
template<class G>
typename G::elem_t apply(const G& g, typename G::elem_t e, const vector<typename G::elem_t>& v) {
    for(auto c: v) e = g.apply(c, e);
    return e;
}

/// Finite (sub)-semigroup generated by elements of <Semigroup> G, as map from elements to generator combinations
template<class G, class V = vector<typename G::elem_t>>
map<typename G::elem_t, vector<int>> spanM(const V& gs, const G& GG = {}) {
    map<typename G::elem_t, vector<int>> M;
    int i = 0;
    for(auto& e: gs) M[e].push_back(i++); // generators already ``found''

    // "newly found" elements, to be combined with generators
    vector<typename G::elem_t> vNew;
    for(auto& x: gs) vNew.push_back(x);

    while(vNew.size()) {
        // unique new combinations
        vector<typename G::elem_t> vv;

        for(auto& e0: vNew) {

            i = 0;
            for(auto& g: gs) { // for each generator
                auto e2 = GG.apply(g,e0);
                auto it = M.find(e2);

                if(it == M.end()) { // new element not previously found?
                    auto g0 = M[e0];
                    g0.push_back(i);
                    M[e2] = g0;
                    vv.push_back(e2);
                }
                ++i;
            }
        }
        vNew = vv;
    }
    return M;
}

/// Construct <Enumerated Semigroup> from iterable list of finite-order generators in <Semigroup> G
template<class SG_t>
class GeneratorsSemigroup: public SG_t {
public:
    /// enumeration type
    typedef size_t enum_t;
    /// Underlying element type
    typedef typename SG_t::elem_t elem_t;
protected:
    vector<elem_t> elems;           ///< enumeration of elements
public:

    /// Constructor, from operator and generators
    explicit GeneratorsSemigroup(const vector<elem_t>& gs, const SG_t& G = {}): SG_t(G), elems(span(gs,G)) { find_id(G); }
    /// Constructor, catchall for alternative generator enumerations
    template<class V>
    explicit GeneratorsSemigroup(const V& gs, const SG_t& G = {}): SG_t(G), elems(span<V>(gs, G)) { find_id(G); }

    /// number of elements in group
    size_t getOrder() const { return elems.size(); }
    /// element index --- BROKEN AFTER RENUMERATION!
    enum_t idx(const elem_t& e) const { assert(!is_renumerated); return std::lower_bound(elems.begin(), elems.end(), e) - elems.begin(); }
    /// indexed element
    const elem_t& element(enum_t i) const { return elems[i]; }
    /// identity element
    const elem_t& identity() const { return element(iID); }
    /// identity element index
    enum_t identity_idx() const { return iID; }
    /// iteration range begin
    auto begin() const -> decltype(elems.begin()) { return elems.begin(); }
    /// iteration range end
    auto end() const -> decltype(elems.end()) { return elems.end(); }


    /// Span of generators in G
    template<class V>
    static vector<elem_t> span(const V& gs, const SG_t& G = {}) {
        auto M = spanM<SG_t>(gs, G);
        vector<elem_t> v;
        for(auto& kv: M) v.push_back(kv.first);
        return v;
    }

    /// apply renumeration
    GeneratorsSemigroup& renumerate(const renumeration_t<enum_t>& m) {
        elems = renumerated_permute(elems,m);
        iID = m.at(iID);
        is_renumerated = true;
        return *this;
    }

protected:
    /// find identity element
    void find_id(const SG_t& G) {
        iID = 0;
        for(auto& e: elems) {
            if(e==G.apply(e,e)) break;
            ++iID;
        }
    }

    bool is_renumerated = false;    ///< whether already renumerated
    enum_t iID = -1;                ///< identity element index

};

/// Cartesian direct product group (G1,G2)
template<class G1, class G2>
class ProductGroup {
public:
    /// element identifier
    typedef pair<typename G1::elem_t, typename G2::elem_t> elem_t;

    /// Get identity element
    static constexpr elem_t identity() { return {G1::identity(), G2::identity()}; }
    /// Get order
    static constexpr size_t getOrder() { return G1::getOrder() * G2::getOrder(); }
    /// Get enumerated element
    static constexpr elem_t element(size_t i) { return {G1::element(i%getOrder()), G2::element(i/getOrder())}; }
    /// Get element inverse
    static constexpr elem_t inverse(elem_t a) { return {G1::inverse(a.first), G2::inverse(a.second)}; }
    /// Get group element c = ab
    static constexpr elem_t apply(elem_t a, elem_t b) { return {G1::apply(a.first,b.first), G2::apply(a.second,b.second)}; }
};

//////////////////////////
// iterator helper classes
//////////////////////////

/// iterator for enumerated semigroup instance
template<class G>
class esg_iterator: public std::iterator<std::forward_iterator_tag, const typename G::elem_t> {
public:
    /// Constructor from grid dimensions
    explicit esg_iterator(const G& g, size_t i = 0): GG(g), c(i) { }
    /// increment
    esg_iterator& operator++() { c++; return *this; }
    /// comparison
    bool operator==(const esg_iterator& rhs) const { return c == rhs.c; }
    /// inequality
    bool operator!=(const esg_iterator& rhs) const { return !(*this == rhs); }
    /// dereference
    const typename G::elem_t& operator*() const { return (e = GG.element(c)); }

protected:
    const G& GG;            ///< underlying group
    size_t c;               ///< current position
    typename G::elem_t e;   ///< current element
};

/// iterator for a static enumerated semigroup class
template<class G>
class esg_static_iterator: public std::iterator<std::forward_iterator_tag, const typename G::elem_t> {
public:
    /// Constructor from grid dimensions
    esg_static_iterator(size_t i = 0): c(i) { if(c < G::getOrder()) e = G::element(c); }
    /// increment
    esg_static_iterator& operator++() { if(++c < G::getOrder()) e = G::element(c); return *this; }
    /// comparison
    bool operator==(const esg_static_iterator& rhs) const { return c == rhs.c; }
    /// inequality
    bool operator!=(const esg_static_iterator& rhs) const { return !(*this == rhs); }
    /// dereference
    const typename G::elem_t& operator*() const { return e; }

protected:
    size_t c;               ///< current position
    typename G::elem_t e{}; ///< current element
};

/*
/// Check for non-empty set (or ordered list) intersection
template<class S>
bool intersects(const S& s1, const S& s2) {
    auto it1 = s1.begin();
    auto it2 = s2.begin();
    while(it1 != s1.end() && it2 != s2.end()) {
        if(*it1 == *it2) return true;
        if(*it1 < *it2) ++it1;
        else ++it2;
    }
    return false;
}
*/

#endif
