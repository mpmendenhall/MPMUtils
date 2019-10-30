/// \file "DecisionTree.hh" Binary decision tree construction and application
// Michael P. Mendenhall, 2019

#ifndef DECISIONTREE_HH
#define DECISIONTREE_HH

#include <algorithm>
#include <stdexcept>
#include <stdio.h>
#include <cassert>

/// Binary decision tree construction and application
class DecisionTree {
public:
    /// Constructor, given N items, M predicates, bool f(i<N, j<M)
    template<typename F>
    DecisionTree(size_t N, size_t M, const F& f);

    /// return categorization of i using decision logic
    template<typename I, typename F>
    size_t decide(const I& i, const F& f) const;

    /// print summary to stdout
    void display(size_t i=0, size_t d=0) const;

protected:

    /// decision tree node
    struct decision_t {
        size_t t = -1;  ///< test number
        size_t a = -1;  ///< branch here if test true; if a == this, halt (this-1)
        size_t b = -1;  ///< branch here if test false; if b == this, halt (this)
        size_t i = -1;  ///< corresponding element
    };
    vector<decision_t> dcs; ///< decision structure

    typedef vector<size_t>::iterator it_t;  ///< convenience typedef

    /// partition range using ``best'' (most even split) operator
    template<typename F>
    size_t rpart(it_t iz, it_t i0, it_t i1, const vector<size_t>& vt, const F& f);
};

////////////////////
////////////////////

template<typename F>
DecisionTree::DecisionTree(size_t N, size_t M, const F& f): dcs(N) {
    if(N <= 1) return;

    vector<size_t> vi(N);
    for(size_t i=0; i<N; i++) vi[i] = i;
    vector<size_t> vt(M);
    for(size_t i=0; i<M; i++) vt[i] = i;

    for(auto& d: dcs) d.a = d.b = 0;
    dcs[0].a = dcs[0].b = rpart(vi.begin(), vi.begin(), vi.end(), vt, f);
    size_t i = 0;
    for(auto e: vi) dcs[i++].i = e;
}

template<typename I, typename F>
size_t DecisionTree::decide(const I& i, const F& f) const {
    if(dcs.size() < 2) return 0;

    size_t d = 0;
    while(true) {
        if(f(i,dcs[d].t)) {
            if(dcs[d].a == d) return dcs[d-1].i;
            d = dcs[d].a;
        } else {
            if(dcs[d].b == d) return dcs[d].i;
            d = dcs[d].b;
        }
    }
}

template<typename F>
size_t DecisionTree::rpart(it_t iz, it_t i0, it_t i1, const vector<size_t>& vt, const F& f) {

    auto N = i1-i0; // number of elements to subdivide

    // choose most even partition point
    size_t bdN = N;
    it_t ix = i0;
    size_t t0 = vt.size();
    for(auto t: vt) {
        ix = std::partition(i0, i1, [&](size_t i) { return f(i,t); });
        size_t dN = abs((i1-ix) - (ix-i0));
        if(dN < bdN) {
            bdN = dN;
            t0 = t;
            if(dN <= 1) break;
        }
    }
    // are we able to subdivide the elements?
    if(bdN >= size_t(N)) throw std::runtime_error("Malformed decision tree");
    // re-calculate partition if we did not end on ``perfect'' result
    if(bdN > 1) ix = std::partition(i0, i1, [&](size_t i) { return f(i,t0); });

    auto sn = ix - iz;  // absolute splitting point
    if(!sn) throw std::runtime_error("Malformed decision tree");
    assert(dcs[sn].t == size_t(-1));
    dcs[sn].t = t0;

    // create sub-nodes
    vector<size_t> subvt;
    for(auto t: vt) if(t != t0) subvt.push_back(t);
    dcs[sn].a = (ix-i0 == 1)? sn : rpart(iz, i0, ix, subvt, f);
    dcs[sn].b = (i1-ix == 1)? sn : rpart(iz, ix, i1, subvt, f);

    return sn;
}

inline void DecisionTree::display(size_t i, size_t d) const {
    if(dcs.size() <= d) { printf("{nondecision}\n"); return; }
    if(dcs.size() == 1) { printf("[0]\n"); return; }
    if(!d) { display(i, dcs[0].a); return; }

    if(dcs[d].a == d) {
        for(size_t j=0; j<i; j++) printf("\t");
        printf("\t[%zu]\n", dcs[d-1].i);
    } else display(i+1, dcs[d].a);

    for(size_t j=0; j<i; j++) printf("\t");
    printf("%zu?\n", dcs[d].t);

    if(dcs[d].b == d) {
        for(size_t j=0; j<i; j++) printf("\t");
        printf("\t[%zu]\n", dcs[d].i);
    } else display(i+1, dcs[d].b);
}

#endif
