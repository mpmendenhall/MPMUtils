/// \file Icosahedral.hh Icosahedral symmetry manipulations
// Michael P. Mendenhall, 2019

#ifndef ICOSAHEDRAL_HH
#define ICOSAHEDRAL_HH

#include "Matrix.hh"
#include "PhiField.hh"
#include "ModularField.hh"
#include "FiniteGroup.hh"

/// Information about icosahedral symmetry point group
namespace Icosahedral {
    using std::array;

    /// Symmetry group element
    typedef Matrix<3,3,PhiField> elem_t;
    /// Group operation (matrix multiplication)
    typedef MultiplySG<elem_t> groupop_t;
    /// Rotation axis type
    typedef Vec<3,PhiField> axis_t;
    /// Vector operated on by group
    typedef Vec<3,SurdSum> vec_t;

    /// triangular rotations enumeration
    typedef ModularField<3>  n3_t;
    /// pentagon rotations enumeration
    typedef ModularField<5>  n5_t;
    /// dodecahedral faces / icosahedral vertices enumeration
    typedef ModularField<12> n12_t;
    /// icosahedral faces = dodecahedral vertices enumeration
    typedef ModularField<20> n20_t;
    /// icosahedral, dodecahedral edges enumeration
    typedef ModularField<30> n30_t;

    // generators for all icosahedral symmetry rotations
    extern const elem_t Ra; ///< one generator
    extern const elem_t Rb; ///< another generator

    /// generators span type
    typedef GeneratorsSemigroup<groupop_t> genspan_t;
    /// All 60 rotation matrices in icosahedral point group
    extern const genspan_t Rs;
    /// Cayley Table type for icosahedral rotations
    typedef CayleyTable<genspan_t> cayley_t;
    /// Precalculated Cayley Table
    extern const cayley_t CT;
    /// Conjugacy classes decomposition type
    typedef ConjugacyDecomposition<cayley_t> conjugacy_t;
    /// Precalculated decomposition
    extern const conjugacy_t CD;

    /// identity element number
    constexpr size_t nID = 0;

    /// dodecahedral face info
    struct f12_t {
        n12_t  i;       ///< enumeration index
        axis_t c;       ///< central axis
        axis_t vs[5];   ///< vertices, clockwise loop
        elem_t R[5];    ///< ID and successive 2*pi/5 clockwise rotations
    };
    extern const array<f12_t,12> dodFaces;

    /// icosahedral face info
    struct f20_t {
        n20_t  i;       ///< enumeration index
        axis_t c;       ///< central axis
        axis_t vs[3];   ///< vertices, clockwise loop
        elem_t R[3];    ///< ID and successive 2*pi/3 clockwise rotations
    };
    extern const array<f20_t,20> icoFaces;

    /// apply all 60 rotations to vector, eliminating duplicates
    vector<vec_t> points(const vec_t& v);

    /// cos theta for rotation
    inline SurdSum cosTheta(const elem_t& M) { return (M.trace()-1)/2; }

    /// print description of icosahedral symmetry to stdout
    void describe();
}

/*
 * Polyhedral groups:
 * tetrahedron isomorphic to A4
 * octohedral (+cube): orientation-preserving chiral isormorphic to S4 (24 elements), full has 48 elements
 * icosohedral (+dodecahedron): isomorphic to A5
 */

#endif
