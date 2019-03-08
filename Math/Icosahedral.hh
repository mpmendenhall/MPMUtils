/// \file Icosahedral.hh Icosahedral symmetry manipulations
// Michael P. Mendenhall, 2019

#ifndef ICOSAHEDRAL_HH
#define ICOSAHEDRAL_HH

#include "Matrix.hh"
#include "SurdField.hh"

/// Information about icosahedral symmetry point group
namespace Icosahedral {
    /// Symmetry group element
    typedef Matrix<3,3,SurdSum> elem_t;
    /// Vector operated on by group
    typedef Vec<3,SurdSum> vec_t;

    // generators for all Rs
    extern const elem_t R10;    ///< one generator
    extern const elem_t R58;    ///< another generator

    /// All 60 rotation matrices in icosahedral point group
    extern const vector<elem_t> Rs;

    extern const SurdSum phi;   ///< golden ratio (sqrt(5)+1)/2;
    extern const SurdSum ihp;   ///< 1/phi = (sqrt(5)-1)/2 = phi-1

    // special unit vectors with reduced symmetries
    extern const vec_t u12; ///< dodecahedral face center
    extern const vec_t u20; ///< icosahedral face center
    extern const vec_t u30; ///< dodecahedral/icosahedral edge center

    /// apply all rotations to vector, eliminating duplicates
    vector<vec_t> points(const vec_t& v);

    /// unit rotation axis for matrix
    vec_t axis(const elem_t& M);
}

#endif
