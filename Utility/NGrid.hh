/// @file NGrid.hh N-dimensional grid generator
/*
 * NGrid.hh, part of the MPMUtils package
 * Copyright (c) 2007-2018 Michael P. Mendenhall
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

#ifndef NGRID_HH
#define NGRID_HH

#include <iterator>
#include <stdexcept>

/// Grid index generation
template<size_t N, typename I = int>
class NGrid {
public:
    /// N-dimensional grid index
    typedef std::array<I,N> idx_t;

    /// Constructor, with grid size
    NGrid(const idx_t& g = idx_t()) { stride[0] = 1; setGrid(g); }

    void setGrid(const idx_t& g) {
        ngrid = g;
        for(size_t i=1; i<N; i++) stride[i] = ngrid[i-1]*stride[i-1];
        size = ngrid[N-1]*stride[N-1];
    }

    /// iterator over grid indices
    class iterator {
    public:
        /// for STL iterator interface
        using iterator_category = std::forward_iterator_tag;
        /// for STL iterator interface
        using value_type = const idx_t;
        /// for STL iterator interface
        using difference_type = std::ptrdiff_t;
        /// for STL iterator interface
        using pointer = value_type*;
        /// for STL iterator interface
        using reference = value_type&;

        /// Constructor from grid dimensions
        explicit iterator(const idx_t& g): ngrid(g), c() { }
        /// Constructor at mid position
        iterator(const idx_t& g, size_t ii): ngrid(g), i(ii), c() {
            if(i==(size_t)-1) a = N;
            // TODO calculate position otherwise
            else throw std::logic_error("unimplemented");
        }

        /// increment
        iterator& operator++() {
            if(a == N) { i = -1; return *this; }
            if(c[a]+1 == ngrid[a]) {
                c[a++] = 0;
                return ++(*this);
            }
            ++c[a];
            a = 0;
            ++i;

            return *this;
        }
        /// comparison
        bool operator==(const iterator& rhs) const { return i == rhs.i && ngrid == rhs.ngrid; }
        /// inequality
        bool operator!=(const iterator& rhs) const { return !(*this == rhs); }
        /// dereference
        const idx_t& operator*() const { return c; }

    protected:
        friend class NGrid;
        idx_t ngrid;    ///< underlying grid
        size_t a = 0;   ///< axis being incremented
        size_t i = 0;   ///< total index; -1 at end of range
        idx_t c;        ///< multidimensional index
    };

    /// start of grid enumeration
    iterator begin() const { return iterator(ngrid); }
    /// end of grid enumeration
    iterator end() const { return iterator(ngrid,-1); }

    /// indexed coordinate position, spanning corners of bounding box (use with BBox.hh)
    template<class BB>
    typename BB::coord_t cornerpos(const idx_t i, const BB& B) const {
        static_assert(N == BB::N, "dimension mismatch");
        typename BB::coord_t c;
        for(size_t a=0; a<N; a++) c[a] = B.pos(ngrid[a] > 1? (typename BB::x_t)(i[a])/(ngrid[a]-1) : 0.5, a);
        return c;
    }

    /// indexed coordinate position, centered in subdivided BBox (use with BBox.hh)
    template<class BB>
    typename BB::coord_t centerpos(const idx_t i, const BB& B) const {
        static_assert(N == BB::N, "dimension mismatch");
        typename BB::coord_t c;
        for(size_t a=0; a<N; a++) c[a] = B.pos(((typename BB::x_t)(i[a])+0.5)/ngrid[a], a);
        return c;
    }

protected:
    size_t size = 0;///< total number of points
    idx_t ngrid;    ///< number of grid points in each dimension
    idx_t stride;   ///< grid stride in each dimension; total size
};

#endif
