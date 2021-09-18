/// \file Partition.hh Partitions (abstract and of arrays)
// -- Michael P. Mendenhall, 2019

#ifndef PARTITION_HH
#define PARTITION_HH

#include <iostream>
#include <cassert>
#include <array>
using std::array;

#if __cplusplus >= 201400L
#define _constexpr constexpr
#else
#define _constexpr
#endif

/// Partitioning of a fixed number of elements
template<size_t N, typename idx_t = size_t>
class Partition: public array<idx_t,N> {
public:
    /// index array type
    typedef array<idx_t,N> idx_array_t;
    /// default constructor
    _constexpr Partition(): idx_array_t{} { (*this)[0] = N; }

    /// partition lower bound
    constexpr inline idx_t i0(idx_t i) const { return i? (*this)[i-1] : 0; }
    /// partition size
    constexpr inline idx_t len(idx_t i) const { return i? (*this)[i]-(*this)[i-1] : (*this)[i]; }

    /// re-order partitioned groups according to specified scheme
    template<typename valarray_t>
    _constexpr Partition reorder(const idx_array_t& o, valarray_t& v) const {
        Partition nn{};
	valarray_t vv{};

        idx_t i=0, jj = 0;
        for(; jj < N; ++i) {
            auto k = o[i];
            for(idx_t j = 0; j < len(k); j++) vv[jj++] = v[i0(k)+j];
            nn[i] = jj;
        }
        assert(jj == N);
        for(; i<N; ++i) nn[i] = nn[i-1]; // finish filling bounds array

        v = vv;
        return nn;
    }

    /// sort order by descending partition length
    constexpr idx_array_t cyclen_descending(size_t nc = N) const {
        auto ci = RangeArray<idx_t,0,N>();
        std::stable_sort(ci.begin(), ci.begin()+nc, [&](idx_t j, idx_t k) { return len(k) < len(j); });
        return ci;
    }
};

/// output representation for partition
template<size_t N, typename idx_t>
std::ostream& operator<<(std::ostream& o, const Partition<N,idx_t>& C) {
    idx_t n = 0;
    o << "|";
    for(auto u: C) {
        for(; n < u; n++) o << '.';
        o << "|";
    }
    return o;
}

/// Partition with data array
template<size_t N, typename val_t, typename idx_t = size_t>
class PartArray: public Partition<N,idx_t> {
public:
    /// partition base class
    typedef Partition<N,idx_t> partition_t;
    /// data array type
    typedef array<val_t,N> val_array_t;

    /// default constructor
    constexpr PartArray(): partition_t{}, v{} { }

    val_array_t v{};  ///< contents

    /// reorder this object's data
    void reorder(const typename Partition<N,idx_t>::idx_array_t& o) { (Partition<N,idx_t>&)(*this) = Partition<N,idx_t>::reorder(o,v); }
    /// sort in "canonical" descending-cycle-length order
    void sort(size_t nc = N) { reorder(this->cyclen_descending(nc)); }
};

/// output representation for PartArray
template<size_t N, typename val_t, typename idx_t>
std::ostream& operator<<(std::ostream& o, const PartArray<N,val_t,idx_t>& C) {
    size_t i = 0, cn = 0;
    while(i<N) {
        assert(cn < N);
        if(cn) o << "|";
        for(size_t j=0; j<C.len(cn); j++) {
            if(j) o << " ";
            o << C.v[i++];
        }
        ++cn;
    }
    return o;
}


#endif
