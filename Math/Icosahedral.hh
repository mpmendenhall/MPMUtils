/// \file Icosahedral.hh Icosahedral symmetry manipulations
// Michael P. Mendenhall, 2019

#ifndef ICOSAHEDRAL_HH
#define ICOSAHEDRAL_HH

#include "Matrix.hh"
#include "SurdField.hh"

/// Information about icosahedral symmetry point group
class IcosahedralSymmetry {
public:
    /// Symmetry group element
    typedef Matrix<3,3,SurdSum> elem_t;
    /// Vector operated on by group
    typedef Vec<3,SurdSum> vec_t;

    /// All 60 rotation matrices in icosahedral point group
    static const vector<elem_t> Rs;

    /// apply all rotations to vector, eliminating duplicates
    static vector<vec_t> points(const vec_t& v);
};

#endif
