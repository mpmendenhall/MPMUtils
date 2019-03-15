/// \file MathieuGroup.hh Mathieu M11,12,21,22 simple finite groups
// Michael P. Mendenhall, 2019

#ifndef MATHIEUGROUP_HH
#define MATHIEUGROUP_HH

#include "FiniteGroup.hh"
#include "Matrix.hh"
#include "ModularField.hh"
#include "PermutationGroup.hh"

/// Information about Mathieu groups M11, M12, M21, M22
namespace MathieuGroup {
    /// Representation element as a matrix
    typedef Matrix<5,5,ModularField<3>> M11_mrepr_t;
    /// Representation element as a permutation
    typedef Permutation<11> M11_repr_t;

    // M11 generators
    extern const M11_mrepr_t M11ma; ///< one matrix generator
    extern const M11_mrepr_t M11mb; ///< another matrix generator
    extern const M11_repr_t M11a;   ///< one permutation generator
    extern const M11_repr_t M11b;   ///< another permutation generator

    /// generators span type
    typedef GeneratorsSemigroup<MultiplySG<M11_repr_t>> M11_genspan_t;

    /// fully enumerated group (static object produced when function first called)
    const M11_genspan_t& M11();

    /// conjugacy group decomposition for M11
    typedef ConjugacyDecomposition<M11_genspan_t> M11_conj_t;
    /// static M11 conjugacy group decomposition
    const M11_conj_t& M11_conj();

    /// Cayley Table type for M11
    typedef CayleyTable<M11_genspan_t> M11_cayley_t;
    /// Precalculated Cayley Table for M11
    const M11_cayley_t& M11_CT();

    //------------------------------

    /// M12 representation element
    typedef Permutation<12> M12_repr_t;

    // M12 generators
    extern const M12_repr_t M12a;   ///< one generator
    extern const M12_repr_t M12b;   ///< another generator

    /// generators span type
    typedef GeneratorsSemigroup<SymmetricGroup<12>> M12_genspan_t;

    /// fully enumerated group (static object produced when function first called)
    const M12_genspan_t& M12();

    //------------------------------

    /// M21 representation element
    typedef Permutation<21> M21_repr_t;

    // M21 generators
    extern const M21_repr_t M21a;   ///< one generator
    extern const M21_repr_t M21b;   ///< another generator

    /// generators span type
    typedef GeneratorsSemigroup<SymmetricGroup<21>> M21_genspan_t;

    /// fully enumerated group (static object produced when function first called)
    const M21_genspan_t& M21();

    //------------------------------

    /// M22 representation element
    typedef Permutation<22> M22_repr_t;

    // M22 generators
    extern const M22_repr_t M22a;   ///< one generator
    extern const M22_repr_t M22b;   ///< another generator

    /// generators span type
    typedef GeneratorsSemigroup<SymmetricGroup<22>> M22_genspan_t;

    /// fully enumerated group (static object produced when function first called)
    const M22_genspan_t& M22();
}

#endif
