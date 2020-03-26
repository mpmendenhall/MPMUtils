/// \file CyclicGroup.hh Cyclic groups
// -- Michael P. Mendenhall, 2019

#ifndef CYCLICGROUP_HH
#define CYCLICGROUP_HH

#include "FiniteGroup.hh"
#include "ModularField.hh"

/// Cyclic group on N elements
template<size_t N>
class CyclicGroup {
public:
    /// element representation
    typedef ModularField<N> elem_t;
    /// enumeration type
    typedef int enum_t;

    /// Get number of elements
    static constexpr size_t getOrder() { return N; }
    /// Get identity element
    static constexpr elem_t identity() { return 0; }
    /// identity element index
    static enum_t identity_idx() { return 0; }
    /// Get index of element
    static constexpr enum_t idx(elem_t i) { return int(i); }
    /// Get enumerated element
    static constexpr elem_t element(enum_t i) { return i; }
    /// Get element inverse
    static constexpr elem_t inverse(elem_t a) { return -a; }
    /// Get group element c = ab
    static constexpr elem_t apply(elem_t a, elem_t b) { return a+b; }
    /// element iteration start
    static constexpr auto begin() { return elem_t::ref_iterator::begin(); }
    /// element iteration end
    static constexpr auto end() { return elem_t::ref_iterator::end(); }
};

#endif
