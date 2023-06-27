/// \file ModularField.hh Modular Integers field

#ifndef MODULARFIELD_HH
#define MODULARFIELD_HH

#include <stdexcept>
#include <cassert>
#include <utility>
using std::pair;
#include <iostream>
#include <cstdint>

/// Euclid's algorithm for relatively prime numbers, returning c,d: c*p = d*q + 1
inline pair<int,int> EuclidRelPrime(int p, int q) {
    if(abs(p)==1) return {p,0};
    if(abs(q)==1) return {0,-q};
    auto qr = std::div(p,q);
    if(qr.rem == 1) return {1, qr.quot};
    auto uv = EuclidRelPrime(q, qr.rem);
    return {-uv.second, -(uv.second*qr.quot + uv.first)};
}

/// precalculated NxN mod-N multiplication table
const uint_fast8_t* modMulTable(size_t N);

/// Modular Integers; default up to N <= 255 fits in uchar
template<size_t N, typename int_t = uint_fast8_t>
class ModularField {
public:
    /// Default constructor to 0
    ModularField(): i(0) { }
    /// Implicit constructor from integer
    ModularField(int n): i(n<0? N-1-(abs(n+1)%N) : n%N) { }

    /// check if 0
    explicit operator bool() const { return  i; }
    /// cast to int
    explicit operator int() const { return i; }
    /// array indexing
    operator size_t() const { return i; }

    /// comparison
    bool operator<(ModularField Z) const { return i < Z.i; }
    /// equality
    bool operator==(ModularField Z) const { return i == Z.i; }
    /// inequality
    bool operator!=(ModularField Z) const { return i != Z.i; }

    /// unary minus
    const ModularField operator-() const { return ModularField(i? N-i : 0, true); }
    /// increment
    ModularField& operator++() { if(++i == int_t(N)) i = 0; return *this; }
    /// decrement
    ModularField& operator--() { if(i) --i; else i = N-1; return *this; }

    /// inplace multiplication
    ModularField& operator*=(ModularField Z) {
        if(N < 256) {
            static const unsigned char* t = modMulTable(N);
            i = t[i*N + Z.i];
        } else i = (i*Z.i)%N;
        return *this;
    }
    /// multiplication
    const ModularField operator*(ModularField Z) const { auto c = *this; return c *= Z; }
    /// contents inverse
    ModularField inverse() { if(!i) throw std::range_error("1/0 is bad!"); return EuclidRelPrime(i, N).first; }
    /// invert this
    ModularField& invert() { return (*this = inverse()); }
    /// inplace division
    ModularField& operator/=(ModularField Z) { return (*this) *= Z.inverse(); }
    /// division
    const ModularField operator/(ModularField Z) const { return (*this) * Z.inverse(); }

    /// inplace addition
    ModularField& operator+=(ModularField Z) {
        if(N > 128) { // avoid overflow of uchar
            int k = int(i)+int(Z.i);
            if(k >= int(N)) k -= N;
            i = k;
        } else {
            i += Z.i;
            if(i >= int(N)) i -= N;
        }
        return *this;
    }
    /// addition
    const ModularField operator+(ModularField Z) const { auto c = *this; return c += Z; }
    /// inplace subtraction
    ModularField& operator-=(ModularField Z) { return *this += -Z; }
    /// subtraction
    const ModularField operator-(ModularField Z) const { return *this + -Z; }

    friend class iterator;

    /// iterator over range N; can use 'for(auto n: ModularField<N>::iterator()) { }'
    class iterator {
    public:
        /// for STL iterator interface
        using iterator_category = std::forward_iterator_tag;
        /// for STL iterator interface
        using value_type = const ModularField;
        /// for STL iterator interface
        using difference_type = std::ptrdiff_t;
        /// for STL iterator interface
        using pointer = value_type*;
        /// for STL iterator interface
        using reference = value_type&;

        /// Constructor
        constexpr iterator(ModularField n = {}): i(n) { }

        /// increment
        iterator& operator++() {
            ++i.i;
            return *this;
        }
        /// comparison
        constexpr bool operator==(const iterator& rhs) const { return i == rhs.i; }
        /// inequality
        constexpr bool operator!=(const iterator& rhs) const { return i != rhs.i; }
        /// dereference
        constexpr ModularField operator*() const { return i; }

        /// start stepping through modular range
        static constexpr iterator begin() { return iterator(); }
        /// stop stepping through modular range
        static constexpr iterator end() { return iterator(ModularField(N,true)); }

    protected:
        ModularField i; ///< internal index, N at range end
    };

    /// reference-based iterator
    class ref_iterator {
    public:
        /// for STL iterator interface
        using iterator_category = std::forward_iterator_tag;
        /// for STL iterator interface
        using value_type = const ModularField;

        /// Constructor
        constexpr ref_iterator(ModularField n = {}): i(n) { }

        /// increment
        ref_iterator& operator++() {
            ++i.i;
            return *this;
        }
        /// comparison
        constexpr bool operator==(const ref_iterator& rhs) const { return i == rhs.i; }
        /// inequality
        constexpr bool operator!=(const ref_iterator& rhs) const { return i != rhs.i; }
        /// dereference
        const ModularField& operator*() const { return i; }

        /// start stepping through modular range
        static constexpr ref_iterator begin() { return ref_iterator(); }
        /// stop stepping through modular range
        static constexpr ref_iterator end() { return ref_iterator(ModularField(N,true)); }

    protected:
        ModularField i; ///< internal index, N at range end
    };

protected:
    /// Special-purpose constructor without modular bounds
    ModularField(int n, bool): i(n) { }

    int_t i;  ///< internal representation in [0,N), or i=N for special iterator end
};

/// output representation for modular number
template<size_t N, typename int_t>
std::ostream& operator<<(std::ostream& o, ModularField<N,int_t> Z) { o << int(Z); return o; }

#endif
