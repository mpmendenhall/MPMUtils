/// \file JankoGroup.hh ``J1'' Janko group structure as test of finite groups manipulation
// Michael P. Mendenhall, 2019

#ifndef JANKOGROUP_HH
#define JANKOGROUP_HH

#include "FiniteGroup.hh"
#include "Matrix.hh"
#include "ModularField.hh"

/// Information about icosahedral symmetry point group
namespace JankoGroup {
    /// Representation element
    typedef Matrix<7,7,ModularField<11>> J1_repr_t;

    // group generators
    extern const J1_repr_t Y;   ///< one generator
    extern const J1_repr_t Z;   ///< another generator

    /// generators span type
    typedef GeneratorsSemigroup<MultiplySG<J1_repr_t>> J1_genspan_t;

    /// fully enumerated group (static object produced when function first called)
    const J1_genspan_t& J1();
}

#endif
