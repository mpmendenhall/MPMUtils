/// \file QuotientGroup.hh Quotient group constructions
// -- Michael P. Mendenhall, 2019

/*
 * ---- Cosets. ----
 * H a subgroup of G: gH = {gh: h in H}, Hg = {hg: h in H} are left- and right-cosets
 * defines equivalence classes x ~ y : x^-1 y in H
 *
 * Index of H in G |G:H| = number of cosets of H in G; |G:H| = |G|/|H| for finite groups
 *
 * -- Normal subgroup -- N of G:
 * gN = Ng for all g in G ("invariant under conjugation", gNg^-1 = N)
 *
 * Abelian G: all subgroups are normal
 *
 * The kernel  ker f = {g in G: f(g) = identity in H}
 * of any group homomorphism f: G -> H is a normal subgroup of G.
 *
 * Quotient group G/N, N normal subgroup of G: "equivalence relation preserving group structure"
 * group operation (aN)*(bN) = abN
 * Equivalence classes = cosets of N (left cosets == right cosets since N normal)
 * Equivalence class of identity is N
 *
 * Set of subgroups form a `complete lattice': every subset has a supremum and infimum
*/

#ifndef QUOTIENTGROUP_HH
#define QUOTIENTGROUP_HH

#include "FiniteGroup.hh"
#include "EquivalenceClasses.hh"

/// Construct <Semigroup> ``quotient'' by equivalence classes
template<class SG_t, class EQ_t>
class EquivalenceSubSG {
public:
    /// Underlying element type: equivalence class identifier
    typedef typename EQ_t::eqidx_t elem_t;

    /// Constructor
    EquivalenceSubSG(const SG_t& g, const EQ_t& e): SG(g), Eq(e) { }

    /// Semigroup operation
    const elem_t apply(const elem_t& a, const elem_t& b) {
        return Eq.classidx(SG.apply(Eq.representative(a), Eq.representative(b)));
    }

protected:
    const SG_t& SG; ///< underlying semigroup
    const EQ_t& Eq; ///< equivalence relation
};

/// Left (right) coset gS (Sg) in group G
template<class G_t, class S_t>
vector<typename G_t::elem_t>
coset(typename G_t::elem_t g,
      const S_t& S,
      const G_t& G,
      bool left = true) {

    vector<typename G_t::elem_t> v;
    for(auto& h: S) v.push_back(left? G.apply(g,h) : G.apply(h,g));
    std::sort(v.begin(), v.end());
    v.erase(std::unique(v.begin(),v.end()),v.end());
    return v;
}

/// Check whether subset is normal in group (gN = Ng for all g in G)
template<class G_t, class SG_t>
bool isNormal(const SG_t& N, const G_t& G) {
    for(auto& g: G)
        if(coset(g,N,G,false) != coset(g,N,G,true))
            return false;
    return true;
}

/// Construct left cosets equivalence classes from group G, subgroup elements set H
template<class EQ_t, class G_t, class SG_t>
void constructLeftCosets(EQ_t& EQ, const G_t& G, const SG_t& H) {
    // x ~ y <==> x^-1 y in H
    auto eq = [&G, &H](const typename G_t::elem_t& a, const typename G_t::elem_t& b) {
            return H.count(G.apply(G.inverse(a), b));
    };
    for(auto& e: G) EQ.classify(e, eq);
}

/// Quotient group G/N, defined by |G:N| = |G|/|N| equivalence classes of cosets of N
template <class G_t>
class QuotientGroup: public EquivalenceSubSG<G_t, EquivalenceClasses<typename G_t::elem_t>> {
public:
    /// Equivalence classes by quotient element
    typedef EquivalenceClasses<typename G_t::elem_t> EQ_t;
    /// Semigroup parent type
    typedef EquivalenceSubSG<G_t, EQ_t> super;
    /// Elements are equivalence class indices
    typedef typename super::elem_t elem_t;

    /// Constructor, from <Enumerated Group> G and normal subgroup elements set S
    template<class NS_t>
    QuotientGroup(const G_t& G, const NS_t& S): super(G, EQ) { constructLeftCosets(EQ, G, S); }

    /// element inverse
    elem_t inverse(elem_t a) const { return EQ.classidx(super::SG.inverse(EQ.representative(a))); }
    /// number of elements (= number of equivalence classes)
    size_t order() const { return EQ.size(); }

    EQ_t EQ;    ///< quotient equivalence classes
};

#endif
