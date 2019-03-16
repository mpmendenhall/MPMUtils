/// \file "FiniteGroup.hh" Base classes for finite groups
// Michael P. Mendenhall, 2019

/*
 * Simple group G: only normal subgroups are {e} and G
 * Normal subgroup N of G: gN = Ng for all g in G ("invariant under conjugation", gNg^-1 = N)
 * Non-simple: can be divided G/H, where S is a subgroup
 * Center of a group Z(G) = elements that commute with all elements in G is a normal subgroup.
 * Centralizer of g in G: set of elements that commute with g
 *
 * Direct products GxH: G, H are normal in GxH
 * Conjugacy classes, centers are Cartesian products of those for G,H
 * if A normal in G, B normal in H, then AxB normal in GxH with (GxH)/(AxB) isomorphic to (G/A)x(H/B)
 *
 * Solvable group: can be constructed from Abelian groups with extensions
 * Every group of odd order is solvable => "every finite simple group has even order unless it is cyclic of prime order."
 */

/////////////////////////
// <Semigroup> interface:
//
// typedef <element> elem_t;                // type for Semigroup element
// elem_t apply(elem_t a, elem_t b) const;  // apply semigroup operator to get c = ab

////////////////////////////////////
// <Enumerated Semigroup> interface:
// <Semigroup> plus:
//
// size_t getOrder() const;             // number of elements
// <iterator> begin() const;            // for iterating elements
// <iterator> end() const;              // for iterating elements
// size_t idx(elem_t) const;            // enumeration index for element
// elem_t element(size_t i) const;      // get enumerated element

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

#include "EquivalenceClasses.hh"
#include <map>
using std::map;
#include <array>
using std::array;
#include <utility>
using std::pair;
#include <vector>
using std::vector;
#include <set>
using std::set;
#include <list>
using std::list;
#include <iostream>
#include <algorithm>
#include <cassert>
#include <numeric>

////////////////////////////////////
////////////////////////////////////
////////////////////////////////////

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
template<class G>
class GeneratorsSemigroup: public G {
public:
    /// Underlying element type
    typedef typename G::elem_t elem_t;

    /// Constructor, from operator and generators
    GeneratorsSemigroup(const vector<elem_t>& gs, const G& GG = {}): G(GG), elems(span(gs,GG)) { }
    /// Constructor, catchall for alternative generator enumerations
    template<class V>
    GeneratorsSemigroup(const V& gs, const G& GG = {}): G(GG), elems(span<V>(gs,GG)) { }

    /// number of elements in group
    size_t getOrder() const { return elems.size(); }
    /// element index
    size_t idx(const elem_t& e) const { return std::lower_bound(elems.begin(), elems.end(), e) - elems.begin(); }
    /// indexed element
    const elem_t& element(size_t i) const { return elems[i]; }
    /// iteration range begin
    auto begin() const { return elems.begin(); }
    /// iteration range end
    auto end() const { return elems.end(); }

    /// Span of generators in G
    template<class V>
    static vector<elem_t> span(const V& gs, const G& GG = {}) {
        auto M = spanM<G>(gs,GG);
        vector<elem_t> v;
        for(auto& kv: M) v.push_back(kv.first);
        return v;
    }

    /// apply renumeration
    GeneratorsSemigroup& renumerate(const renumeration_t<>& m) { elems = renumerated_permute(elems,m); return *this; }

protected:
    vector<elem_t> elems; ///< enumeration of elements
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

/// iterator for integers 0...N-1
template<typename val_t = size_t>
class range_iterator: public std::iterator<std::forward_iterator_tag, const val_t> {
public:
    /// Constructor from total size, starting point
    range_iterator(val_t n, val_t i = {}): N(n), c(i) { }
    /// increment
    range_iterator& operator++() { c++; return *this; }
    /// comparison
    bool operator==(const range_iterator& rhs) const { return c == rhs.c; }
    /// inequality
    bool operator!=(const range_iterator& rhs) const { return !(*this == rhs); }
    /// dereference
    const val_t& operator*() const { return c; }

protected:
    const val_t N; ///< maximum
    val_t c;       ///< current value
};

/// iterator for enumerated semigroup instance
template<class G>
class esg_iterator: public std::iterator<std::forward_iterator_tag, const typename G::elem_t> {
public:
    /// Constructor from grid dimensions
    esg_iterator(const G& g, size_t i = 0): GG(g), c(i) { }
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
class esg_siterator: public std::iterator<std::forward_iterator_tag, const typename G::elem_t> {
public:
    /// Constructor from grid dimensions
    esg_siterator(size_t i = 0): c(i) { if(c < G::getOrder()) e = G::element(c); }
    /// increment
    esg_siterator& operator++() { if(++c < G::getOrder()) e = G::element(c); return *this; }
    /// comparison
    bool operator==(const esg_siterator& rhs) const { return c == rhs.c; }
    /// inequality
    bool operator!=(const esg_siterator& rhs) const { return !(*this == rhs); }
    /// dereference
    const typename G::elem_t& operator*() const { return e; }

protected:
    size_t c;               ///< current position
    typename G::elem_t e{}; ///< current element
};

///////////////////////////////
// group analysis/decomposition
///////////////////////////////

/// Construct <Enumerated Semigroup> Cayley Table isomorphism of input <Enumerated Semigroup> G for faster group operations
template<class G>
class CayleyTable {
public:
    /// Enumerated element type
    typedef size_t elem_t;

    /// Constructor, from underlying <Enumerated Semigroup>
    CayleyTable(const G& GG = {}, bool pb = false): order(GG.getOrder()), CT(buildCT(GG,pb)) { }

    /// pre-calculated group operator
    elem_t apply(elem_t a, elem_t b) const { auto it = CT.find({a,b}); assert(it != CT.end()); return it->second; }
    /// get group order
    size_t getOrder() const { return order; }

    /// return (trivial!) element index
    static constexpr size_t idx(elem_t i) { return i; }
    /// indexed element
    static constexpr elem_t element(size_t i) { return i; }
    /// element iteration start
    auto begin() const { return range_iterator<elem_t>(order); }
    /// element iteration end
    auto end() const { return range_iterator<elem_t>(order,order); }

    /// apply renumeration of elements
    CayleyTable& renumerate(const renumeration_t<elem_t>& m) {
        map<pair<elem_t,elem_t>, elem_t> M;
        for(auto& kv: CT) M.emplace(pair<elem_t,elem_t>(m.at(kv.first.first), m.at(kv.first.second)), m.at(kv.second));
        CT = M;
        return *this;
    }

protected:
    const size_t order;                     ///< number of elements
    map<pair<elem_t,elem_t>, elem_t> CT;    ///< Cayley Table ab -> c

    /// build Cayley Table from all element pairs
    decltype(CT) buildCT(const G& GG, bool pb) {
        map<pair<elem_t,elem_t>, elem_t> m;
        elem_t i = 0;
        for(auto& e1: GG) {
            if(pb && !(i%(GG.getOrder()/100))) std::cout << "*" << std::flush;
            elem_t j = 0;
            for(auto& e2: GG) {
                auto k = GG.idx(GG.apply(e1,e2));
                assert(k < GG.getOrder());
                m[{i,j++}] = k;
            }
            ++i;
        }
        if(pb) std::cout << std::endl;
        return m;
    }
};

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

/// Analysis of <Enumerated Semigroup> into element orders
template<class G>
class OrdersDecomposition {
public:
    /// Constructor from <Enumerated Semigroup>
    OrdersDecomposition(const G& g = {}) {
        cycles.resize(g.getOrder());
        size_t i = 0;
        for(auto& e: g) {
            auto& v = cycles[i++];
            auto f = e;
            do {
                v.push_back(g.idx(f));
                f = g.apply(e, f);
            } while(f != e);
        }

        for(i = 0; i < cycles.size(); i++) by_order[cycles[i].size()].insert(i);
    }

    /// Display info
    void display(std::ostream& o = std::cout) {
        o << "Group with " << cycles.size() << " elements:\n";
        for(auto& kv: by_order) o << "\t" << kv.second.size() << " elements of order " << kv.first << "\n";
    }

    map<size_t,set<size_t>> by_order;       ///< elements (by index) grouped by order
    vector<vector<size_t>> cycles;          ///< cyclic groups generated by each element

    /// apply renumeration
    OrdersDecomposition& renumerate(const renumeration_t<>& m) {
        for(auto& kv: by_order) kv.second = renumerated(kv.second, m);
        cycles = renumerated_permute(cycles,m);
        return *this;
    }
};

/// Analysis of <Enumerated Semigroup> into element orders and conjugacy classes
template<class G>
class ConjugacyDecomposition: public OrdersDecomposition<G> {
public:
    /// Constructor from <Enumerated Semigroup>
    ConjugacyDecomposition(const G& g = {}): OrdersDecomposition<G>(g) {
        for(auto& x: g) vscramble.push_back(x);
        std::random_shuffle(vscramble.begin(), vscramble.end());

        for(auto& os: this->by_order) {
            for(auto i: os.second) assign_CC(os.first, i, g);

            if(g.getOrder() > 1000) {
                std::cout << "Order " << os.first << ": ";
                for(auto& cc: M[os.first].CCs) std::cout << "(" << cc.second.size() << ") ";
                std::cout << std::endl;
            }

            repr_cosets.erase(os.first);
        }

        vscramble.clear();
    }

    /// Display info
    void display(std::ostream& o = std::cout) const {
        o << "Group with " << this->cycles.size() << " elements in conjugacy classes:\n";
        for(auto& kv: M)
            for(auto& ec: kv.second.CCs)
                o << "\t" << ec.second.size() << " elements\t[order " << kv.first << "]\n";
    }

    /// ``nicer'' re-enumeration scheme
    renumeration_t<> make_renumeration() const {
        renumeration_t<> m;
        size_t i = 0;
        for(auto& kv: M)
            for(auto& ec: kv.second.CCs)
                for(auto& e: ec.second)
                    m[e] = i++;
        return m;
    }

    /// apply renumeration
    ConjugacyDecomposition& renumerate(const renumeration_t<>& m) {
        OrdersDecomposition<G>::renumerate(m);
        for(auto& kv: M) kv.second.CCs.renumerate(m);
        return *this;
    }

    /// information on elements of a particular order
    struct oinfo {
        /// Conjugacy class decomposition for elements of this order
        EquivalenceClasses<size_t> CCs;
        /// powerup structure {order, conjclass} for each conjugacy class of this order
        vector<vector<pair<size_t,size_t>>> powerup;
    };
    map<size_t, oinfo> M;   ///< information by order

protected:
    vector<typename G::elem_t> vscramble;           ///< scrambled search order
    map<size_t,vector<vector<size_t>>> repr_cosets; ///< representative cosets for each class --- cleared after construction

    /// determine conj. class number to which element idx=i of order o belongs
    size_t assign_CC(size_t o, size_t i, const G& g) {
        auto& oi = M[o]; // already-identified conjugacy info for this order

        // already categorized?
        auto p = oi.CCs(i);
        if(p.first) return p.second;

        // check if element fits into any existing conjugacy class
        auto e = g.element(i);
        if(oi.CCs.size()) {
            size_t j = 0;
            for(auto& x: vscramble) {
                auto ixe = g.idx(g.apply(x, e));
                for(auto& CC: oi.CCs) {
                    if(ixe != repr_cosets[o][CC.first][j]) continue;

                    auto& pu = oi.powerup[CC.first];
                    if(pu.size()) {
                        // bulk assign all powers of this element if powerup structure already determined
                        size_t k = 0;
                        for(auto ii: this->cycles[i]) {
                            auto oc = pu[k++];
                            M[oc.first].CCs.addTo(ii, oc.second);
                        }
                    } else oi.CCs.addTo(i, CC.first); // only add this element

                    return CC.first;
                }
                j++;
            }
        }

        // start new conjugacy class
        auto n = oi.CCs.add(i,i);

        // build representative coset for class
        auto& vcs = repr_cosets[o];
        vcs.emplace_back();
        auto& cs = vcs.back();
        for(auto& x: vscramble) cs.push_back(g.idx(g.apply(e, x)));

        // build powerup structure
        assert(n == oi.powerup.size());
        oi.powerup.emplace_back();
        vector<pair<size_t,size_t>> pu(1, {o,n});
        for(auto ii: this->cycles[i]) {
            if(ii == i) continue;
            auto oo = this->cycles[ii].size();
            pu.emplace_back(oo, assign_CC(oo,ii,g));
        }
        oi.powerup[n] = pu;

        return n;
    }
};

/// Bundle of calculations resulting in conjugacy-enumerated elements
template<class G>
class GeneratorsConjugacy {
public:
    /// generators span type
    typedef GeneratorsSemigroup<G> genspan_t;
    /// Cayley Table type
    typedef CayleyTable<genspan_t> cayley_t;
    /// Conjugacy classes decomposition type
    typedef ConjugacyDecomposition<cayley_t> conjugacy_t;

    /// Constructor
    GeneratorsConjugacy(const vector<typename G::elem_t>& gs, const G& GG = {}): Rs(gs,GG) {
        auto renum = CD.make_renumeration();
        Rs.renumerate(renum);
        CT.renumerate(renum);
        CD.renumerate(renum);
    }

    genspan_t Rs;       ///< generators span
    cayley_t CT{Rs};    ///< Cayley Table
    conjugacy_t CD{CT}; ///< Conjugacy relations
};


///////////////
// Some Groups.
///////////////

/// Cyclic group on N elements
template<size_t N>
class CyclicGroup {
public:
    /// element representation
    typedef int elem_t;

    /// Get number of elements
    static constexpr size_t getOrder() { return N; }
    /// Get identity element
    static constexpr elem_t identity() { return 0; }
    /// Get index of element
    static constexpr size_t idx(elem_t i) { return i; }
    /// Get enumerated element
    static constexpr elem_t element(size_t i) { return i; }
    /// Get element inverse
    static constexpr elem_t inverse(elem_t a) { return (N-a)%N; }
    /// Get group element c = ab
    static constexpr elem_t apply(elem_t a, elem_t b) { return (a+b)%N; }
    /// element iteration start
    static constexpr auto begin() { return range_iterator<elem_t>(N); }
    /// element iteration end
    static constexpr auto end() { return range_iterator<elem_t>(N,N); }
};

#endif
