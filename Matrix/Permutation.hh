/* 
 * Permutation.hh, part of the RotationShield program
 * Copyright (c) 2007-2014 Michael P. Mendenhall
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/// \file "Permutation.hh" \brief Permutation matrices
#ifndef PERMUTATION_HH
/// Make sure this header is loaded only once
#define PERMUTATION_HH

#include <stdlib.h>
#include <vector>
#include <cassert>

/// Permutation matrix (each row and column has one entry which is 1, all others 0)
class Permutation {
public:
    /// Constructor
    Permutation(unsigned int n);
    /// Destructor
    ~Permutation() {}
    
    /// get size
    unsigned int size() const { return data.size(); }
    /// immutable element access
    unsigned int operator[](unsigned int n) const { assert(n<size()); return data[n]; }
    /// mutable element access
    unsigned int& operator[](unsigned int n) { assert(n<size()); return data[n]; }
    
    /// Set to permutation matrix needed by ShieldPhysics
    Permutation& nshuffle(int n);
    
    /// Permute two elements
    void swap(unsigned int a, unsigned int b);
    
    /// Return the inverse (=transpose) of the matrix
    Permutation inverse() const;
    /// Return the transpose (=inverse) of the matrix
    Permutation transposed() const { return inverse(); }
    /// Invert (=transpose) the matrix inplace
    Permutation& invert();
    /// Transpose (=invert) the matrix inplace
    Permutation& transpose() {return invert(); }
    
    /// Multiply another Permutation to the right
    const Permutation operator*(const Permutation& p) const;
    
private:
    std::vector<unsigned int> data; ///< the permutation data
};

#endif
