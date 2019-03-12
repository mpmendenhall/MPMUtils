/// \file MathieuGroup.hh Mathieu M11 finite group
// Michael P. Mendenhall, 2019

#ifndef MATHIEUGROUP_HH
#define MATHIEUGROUP_HH

#include "FiniteGroup.hh"
#include "Matrix.hh"
#include "ModularField.hh"

/// Information about icosahedral symmetry point group
namespace MathieuGroup {
    /// Representation element
    typedef Matrix<5,5,ModularField<3>> M11_repr_t;

    // group generators
    extern const M11_repr_t a;  ///< one generator
    extern const M11_repr_t b;  ///< another generator

    /// generators span type
    typedef GeneratorsSemigroup<MultiplySG<M11_repr_t>> M11_genspan_t;

    /// fully enumerated group (static object produced when function first called)
    const M11_genspan_t& M11();

    /// Cayley Table type for M11
    typedef CayleyTable<M11_genspan_t> M11_cayley_t;
    /// Precalculated Cayley Table for M11
    const M11_cayley_t& M11_CT();
}

#endif
