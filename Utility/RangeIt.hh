/// \file RangeIt.hh Templatized convenience class for compile-time range generation
// Michael P. Mendenhall, 2019

#ifndef RANGEIT_HH
#define RANGEIT_HH

#include <iterator>
#include <array>
using std::array;

/// iterator for values N0, ... , N1
template<typename val_t, val_t N0, val_t N1>
class RangeIt: public std::iterator<std::forward_iterator_tag, const val_t> {
public:
    /// Constructor, for start or end of range
    constexpr RangeIt(bool start = true): i(start? N0 : N1) { }

    /// increment
    RangeIt& operator++() { assert(i != N1); ++i; return *this; }
    /// comparison
    bool operator==(const RangeIt& rhs) const { return i == rhs.i; }
    /// inequality
    bool operator!=(const RangeIt& rhs) const { return !(*this == rhs); }
    /// dereference
    const val_t& operator*() const { return i; }
    /// dereference and increment for generator use
    val_t operator()() const { return i++; }

    /// range start
    static constexpr RangeIt begin() { return RangeIt(true); }
    /// range end
    static constexpr RangeIt end() { return RangeIt(false); }

protected:

    val_t i;    ///< current value
};

/// pre-populated array
template<typename val_t, val_t N0, val_t N1>
constexpr array<val_t, N1-N0> RangeArray() {
    array<val_t, N1-N0> a{};
    std::copy(RangeIt<val_t,N0,N1>::begin(), RangeIt<val_t,N0,N1>::end(), a.begin());
    return a;
}

/// Variable-range iteration
template<typename val_t = size_t>
class VRangeIt: public std::iterator<std::forward_iterator_tag, const val_t> {
public:
    /// Constructor from total size, starting point
    VRangeIt(val_t n, val_t i = {}): N(n), c(i) { }
    /// increment
    VRangeIt& operator++() { ++c; return *this; }
    /// comparison
    bool operator==(const VRangeIt& rhs) const { return c == rhs.c; }
    /// inequality
    bool operator!=(const VRangeIt& rhs) const { return !(*this == rhs); }
    /// dereference
    const val_t& operator*() const { return c; }
    /// dereference and increment for generator use
    val_t operator()() const { return c++; }

protected:
    const val_t N; ///< maximum
    val_t c;       ///< current value
};

#endif
