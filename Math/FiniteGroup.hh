/// \file "FiniteGroup.hh" Base classes for finite groups
// Michael P. Mendenhall, LLNL 2018

#ifndef FINITEGROUP_HH
#define FINITEGROUP_HH

#include <map>
using std::map;
#include <array>
using std::array;
#include <utility>
using std::pair;
#include <vector>
using std::vector;

/// Product group (G1,G2), and prototype for group classes
template<class G1, class G2>
class ProductGroup {
public:
    /// Number of elements
    static constexpr size_t order = G1::order * G2::order;
    /// element representation
    typedef pair<typename G1::elem_t, typename G2::elem_t> elem_t;

    /// Get identity element
    static constexpr elem_t identity() { return {G1::identity(), G2::identity()}; }
    /// Get enumerated element
    static constexpr elem_t element(size_t i) { return {G1::element(i%order), G2::element(i/order)}; }
    /// Get element inverse
    static constexpr elem_t invert(elem_t a) { return {G1::invert(a.first), G2::invert(a.second)}; }
    /// Get group element c = ab
    static constexpr elem_t apply(elem_t a, elem_t b) { return {G1::apply(a.first,b.first), G2::apply(a.second,b.second)}; }
};

/// Cyclic group on N elements
template<size_t N>
class CyclicGroup {
public:
    /// Number of elements
    static constexpr size_t order = N;
    /// element representation
    typedef int elem_t;

    /// Get identity element
    static constexpr elem_t identity() { return 0; }
    /// Get enumerated element
    static constexpr elem_t element(size_t i) { return i; }
    /// Get element inverse
    static constexpr elem_t invert(elem_t a) { return (N-a)%N; }
    /// Get group element c = ab
    static constexpr elem_t apply(elem_t a, elem_t b) { return (a+b)%N; }
};

/// Compile-time-evaluable factorial function
constexpr size_t factorial(size_t i) { return i > 1? i*factorial(i-1) : 1; }

/// Apply permutation p to array A
template<class A, class P>
A permute(const A& v, const P& p) {
    size_t j=0;
    auto vv = v;
    for(auto i: p) vv[j++] = v[i];
    return vv;
}

/// Symmetric Group (permutations) of N elements
template<size_t N>
class SymmetricGroup {
public:
    /// Number of elements
    static constexpr size_t order = factorial(N);
    /// Permutation representation
    typedef array<int,N> elem_t;

    /// Compile-time calculation of permutation number i of N!
    static constexpr elem_t element(size_t i) {
        elem_t p = identity();
        if(!i) return p;

        auto nsub = factorial(N-1);
        auto j = i/nsub;
        if(j) std::swap(p[j-1], p[N-1]);
        return permute(p, SymmetricGroup<N-1>::element(i%nsub));
    }

    /// identity element
    static constexpr elem_t identity() {
        elem_t p{};
        int i = 0;
        for(auto& x: p) x = i++;
        return p;
    }

    /// Get element inverse
    static constexpr elem_t invert(elem_t a) {
        elem_t e{};
        int j = 0;
        for(auto i: a) e[i] = j++;
        return e;
    }

    /// Get group element c = ab
    static constexpr elem_t apply(elem_t a, elem_t b) { return permute(a,b); }
};
/// Null permutation special case
template<>
SymmetricGroup<0>::elem_t SymmetricGroup<0>::element(size_t) { return {}; }

/// Alternating Group (even permutations) of N elements
template<size_t N>
class AlternatingGroup {
public:
    /// Number of elements
    static constexpr size_t order = (factorial(N)+1)/2;
    /// Permutation representation
    typedef array<int,N> elem_t;

    /// Compile-time calculation of permutation number i of _N!
    static constexpr elem_t element(size_t i) { return SymmetricGroup<N>::element(2*i); }

    /// identity element
    static constexpr elem_t identity() { return SymmetricGroup<N>::identity(); }

    /// Get element inverse
    static constexpr elem_t invert(elem_t a) { return SymmetricGroup<N>::invert(a); }

    /// Get group element c = ab
    static constexpr elem_t apply(elem_t a, elem_t b) { return permute(b,a); }
};

/*
 * Polyhedral groups:
 * tetrahedron isomorphic to A4
 * octohedral (+cube): isormorphic to S4
 * icosohedral (+dodecahedron): isomorphic to A5
 */

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

/// All unique elements produced as combinations of input elements
template<class G>
constexpr map<typename G::elem_t, vector<int>> span(const vector<typename G::elem_t>& gs) {
    map<typename G::elem_t, vector<int>> M;
    int i = 0;
    for(auto& e: gs) M[e].push_back(i++);

    auto vNew = gs;
    while(vNew.size()) {
        vector<typename G::elem_t> vv;
        for(auto& e0: vNew) {
            i = 0;
            for(auto& e1: gs) {
                auto e2 = G::apply(e1,e0);
                auto it = M.find(e2);
                if(it == M.end()) {
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

#endif
