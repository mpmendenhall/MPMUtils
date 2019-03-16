/// \file Partition.hh Partitioning of elements
// Michael P. Mendenhall, 2019

#ifndef PARTITION_HH
#define PARTITION_HH

#include <iostream>
#include <array>
using std::array;

/// Partitioning of a fixed number of elements
template<size_t N, typename idx_t = size_t>
class Partition {
public:
    /// index array type
    typedef array<idx_t,N> idx_array_t;
    idx_array_t n;  ///< upper bound of each partition

    /// partition lower bound
    inline idx_t i0(idx_t i) const { return i? n[i-1] : 0; }
    /// partition size
    inline idx_t len(idx_t i) const { return i? n[i]-n[i-1] : n[i]; }

    /// re-order partitioned groups according to specified scheme
    template<typename valarray_t>
    void reorder(const idx_array_t& o, valarray_t& v) {
        idx_array_t nn;
        valarray_t vv;

        idx_t i=0, jj = 0;
        for(; jj < N; ++i) {
            auto k = o[i];
            for(idx_t j = 0; j < len(k); j++) vv[jj++] = v[i0(k)+j];
            nn[i] = jj;
        }
        for(; i<N; ++i) n[i] = n[i-1]; // finish filling bounds array

        n = nn;
        v = vv;
    }
};

/// Partition with data array
template<size_t N, typename val_t, typename idx_t = size_t>
class PartArray: public Partition<N,idx_t> {
public:
    /// data array type
    typedef array<val_t,N> val_array_t;
    val_array_t v;  ///< contents

    void reorder(const typename Partition<N,idx_t>::idx_array_t& o) { Partition<N,idx_t>::reorder(o,v); }
};

/// output representation for partitions
template<size_t N, typename val_t, typename idx_t>
std::ostream& operator<<(std::ostream& o, const PartArray<N,val_t,idx_t>& C) {
    size_t i = 0, cn = 0;
    while(i<N) {
        assert(cn < N);
        o << "(";
        for(size_t j=0; j<C.len(cn); j++) {
            if(j) o << " ";
            o << C.v[i++];
        }
        o << ")";
        ++cn;
    }
    return o;
}


#endif
