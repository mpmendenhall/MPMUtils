/// \file ModularField.hh Modular Integers field

#ifndef MODULARFIELD_HH
#define MODULARFIELD_HH

#include <stdexcept>
#include <cassert>

/// Euclid's algorithm for relatively prime numbers, returning c,d: c*p = d*q + 1
constexpr inline pair<int,int> EuclidRelPrime(int p, int q) {
    if(abs(p)==1) return {p,0};
    if(abs(q)==1) return {0,-q};
    auto qr = std::div(p,q);
    if(qr.rem == 1) return {1, qr.quot};
    auto uv = EuclidRelPrime(q, qr.rem);
    return {-uv.second, -(uv.second*qr.quot + uv.first)};
}

/// Modular Integers field with N elements
template<size_t N>
class ModularField {
public:
    /// Default constructor to 0
    ModularField(): i(0) { }
    /// Constructor from integer
    ModularField(int n): i((n<0? N*(-n/N + 1) + n : n)%N) { }

    /// check if 0
    explicit operator bool() const { return  i; }
    /// cast to int
    explicit operator int() const { return i; }

    /// comparison
    bool operator<(const ModularField& Z) const { return i < Z.i; }
    /// equality
    bool operator==(const ModularField& Z) const { return i == Z.i; }
    /// inequality
    bool operator!=(const ModularField& Z) const { return i != Z.i; }

    /// unary minus
    const ModularField operator-() const { return ModularField(i? N-i : 0, true); }
    /// increment
    ModularField& operator++() { if(++i == N) i=0; return *this; }
    /// decrement
    ModularField& operator--() { if(--i < 0) i = N-1; return *this; }

    /// inplace multiplication
    ModularField& operator*=(ModularField Z) { i = (i*Z.i)%N; return *this; }
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
    ModularField& operator+=(ModularField Z) { i += Z.i; if(i >= int(N)) i -= N; return *this; }
    /// addition
    const ModularField operator+(ModularField Z) const { auto c = *this; return c += Z; }
    /// inplace subtraction
    ModularField& operator-=(ModularField Z) { return *this += -Z; }
    /// subtraction
    const ModularField operator-(ModularField Z) const { return *this + -Z; }

    friend class iterator;

    /// iterator over range N; can use 'for(auto n: ModularField<N>::iterator()) { }'
    class iterator: public std::iterator<std::forward_iterator_tag, const ModularField> {
    public:
        /// Constructor
        constexpr iterator(ModularField n = {}): i(n) { }

        /// increment
        iterator& operator++() {
            assert(i.i < N);
            ++i.i;
            return *this;
        }
        /// comparison
        constexpr bool operator==(const iterator& rhs) const { return i == rhs.i; }
        /// inequality
        constexpr bool operator!=(const iterator& rhs) const { return i != rhs.i; }
        /// dereference
        constexpr ModularField operator*() const { assert(0 <= i.i && i.i < N); return i; }

        /// start stepping through modular range
        constexpr iterator begin() const { return iterator(); }
        /// stop stepping through modular range
        constexpr iterator end() const { return iterator(ModularField(N,true)); }

    protected:
        ModularField i; ///< internal index, N at range end
    };

protected:
    /// Special-purpose constructor without modular bounds
    ModularField(int n, bool): i(n) { }

    int i;  ///< internal representation in [0,N), or i=N for special iterator end
};

/// output representation for modular number
template<size_t N>
std::ostream& operator<<(std::ostream& o, const ModularField<N>& Z) { o << int(Z); return o; }

#endif
