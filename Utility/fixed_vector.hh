/// \file fixed_vector.hh Fixed-size (non-reallocating) vector with std::vector API
// -- Michael P. Mendenhall, LLNL 2022

#ifndef FIXED_VECTOR
#define FIXED_VECTOR

#include <vector>
using std::vector;
#include <stdexcept>

/// Fixed-size (non-reallocating) vector with std::vector API
template <class T, class A = std::allocator<T>>
class fixed_vector : protected std::vector<T, A> {
public:
    /// underlying vector type
    typedef vector<T,A> vec_t;

    using vec_t::vector;

    using vec_t::operator[];
    using vec_t::at;
    using vec_t::size;
    using vec_t::empty;

    using vec_t::front;
    using vec_t::back;
    using vec_t::data;

    using vec_t::begin;
    using vec_t::cbegin;
    using vec_t::rbegin;
    using vec_t::end;
    using vec_t::cend;
    using vec_t::rend;

    /// assign values, with size check
    template <class InputIterator>
    void assign (InputIterator first, InputIterator last) {
        if(size_t(last - first) != size()) throw std::logic_error("Incorrect assignment size");
        vec_t::assign(first, last);
    }

    /// assignment operator
    template<class V>
    fixed_vector& operator=(const V& v) { assign(v.begin(), v.end()); return *this; }
};

#endif
