/// \file JankoGroup.hh ``J1'' Janko group structure as test of finite groups manipulation
// -- Michael P. Mendenhall, 2019

#ifndef JANKOGROUP_HH
#define JANKOGROUP_HH

#include "FiniteGroup.hh"
#include "Matrix.hh"
#include "ModularField.hh"
#include "PermutationGroup.hh"

/// Information about the Janko J1 group
namespace JankoGroup {
    /// Representation as a matrix over integers mod 11
    typedef Matrix<7,7,ModularField<11>> J1_mrepr_t;
    /// Representation as a permutation (~15% faster)
    typedef Permutation<266> J1_prepr_t;

    // group generators
    extern const J1_mrepr_t mY; ///< one matrix generator
    extern const J1_mrepr_t mZ; ///< another matrix generator
    extern const J1_prepr_t pY; ///< one permutation generator
    extern const J1_prepr_t pZ; ///< another permutation generator

    /// matrix generators span type
    typedef GeneratorsSemigroup<MultiplyG<J1_mrepr_t>> J1_mgenspan_t;
    /// permutation generators span type
    typedef GeneratorsSemigroup<SymmetricGroup<266>> J1_pgenspan_t;

    /// fully enumerated group (static object produced when function first called)
    const J1_pgenspan_t& J1p();

    /// fully enumerated group (static object produced when function first called)
    const J1_mgenspan_t& J1m();
}

#endif
