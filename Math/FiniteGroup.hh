/// \file "FiniteGroup.hh" Base classes for finite groups
// Michael P. Mendenhall, LLNL 2018

#ifndef FINITEGROUP_HH
#define FINITEGROUP_HH

#include <map>
using std::map;
#include <array>
using std::array;
#include <stdio.h>

/// Print Cayley Table for generic finite group
template<class G>
void displayGroup() {
    printf("Finite group with %zu elements\n", G::N);
    for(size_t i=0; i<G::N; i++) {
        for(size_t j=0; j<G::N; j++) printf("\t%i", G::apply(i,j));
        printf("\n");
    }
}

/// Product group (G1,G2)
template<class G1, class G2>
class ProductGroup {
public:
    /// Number of elements
    static constexpr size_t N = G1::N * G2::N;

    /// enumeration for subelement (a,b)
    static inline int elnum(int a, int b) { return a + b*G1::N; }
    /// Get element inverse
    static inline int invert(int a) { return elnum(G1::invert(a%G1::N), G2::invert(a/G1::N)); }
    /// Get group element c = ab
    static inline int apply(int a, int b) { return elnum(G1::apply(a%G1::N, b%G1::N), G2::apply(a/G1::N, b/G1::N)); }
};

/// Cyclic group on N elements
template<size_t _N>
class CyclicGroup {
public:
    /// Number of elements
    static constexpr size_t N = _N;
    /// Get element inverse
    static inline int invert(int a) { return N-a; }
    /// Get group element c = ab
    static inline int apply(int a, int b) { return (a+b)%N; }
};

/// Compile-time-evaluable factorial function
constexpr size_t factorial(size_t i) { return i? i*factorial(i-1) : 1; }

/// Compile-time Heap's Algorithm permutation #i/n! on first n elements of A
template<class Arr>
constexpr Arr PermuteArray(int i, int n, Arr A) {
    if(n <= 1) return A; // permutation on 1 element is easy!

    auto nsub = factorial(n-1); // sub-perms on n-1 elements
    auto j = i/nsub;
    if(j) std::swap(A[j-1], A[n-1]);
    return PermuteArray(i%nsub, n-1, A);
}

/// Symmetric Group (permutations) of N elements
template<size_t _N>
class SymmetricGroup {
    /// Number of elements
    static constexpr size_t N = factorial(_N);
    /// Permutation representation
    typedef array<int,_N> perm;

    /*
    /// get i^th permutation
    static const perm& getPerm(int i) {
    }

    /// enumeration for subelement (a,b)
    static inline int elnum(int a, int b) { return a + b*G1::N; }
    /// Get element inverse
    static inline int invert(int a) { return elnum(G1::invert(a%G1::N), G2::invert(a/G1::N)); }
    /// Get group element c = ab
    static inline int apply(int a, int b) { return elnum(G1::apply(a%G1::N, b%G1::N), G2::apply(a/G1::N, b/G1::N)); }
    */
};



#endif
