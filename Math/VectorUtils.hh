/// \file VectorUtils.hh Templatized vector arithmetic utilities
// -- Michael P. Mendenhall, 2020

#ifndef VECTORUTILS_HH
#define VECTORUTILS_HH

#include <type_traits>
#include <algorithm>// for std::for_each
#include <numeric>  // for std::accumulate
#include <cmath>    // for sqrt

/// Get type for contents of fixed array, std::array, std::vector
template<typename T>
using array_contents_t = typename std::remove_reference<decltype(std::declval<T&>()[0])>::type;

/// Get type for *begin()
template<typename T>
using atbegin_t = typename std::remove_reference<decltype(*std::declval<T&>().begin())>::type;

/// element-wise negation
template<typename V>
void negate(V& v) { std::for_each(v.begin(), v.end(), [](atbegin_t<V>& x) { x = -x; }); }

/// element-wise negation with offset
template<typename V>
void negate(V& v, array_contents_t<V> c) { std::for_each(v.begin(), v.end(), [=](atbegin_t<V>& x) { x = c-x; }); }

/// scalar multiplication v *= s
template<typename V, typename T>
inline void scale(V& v, const T& s) { std::for_each(v.begin(), v.end(), [&s](atbegin_t<V>& x) { x *= s; }); }

/// scalar division v /= s
template<typename V, typename T>
inline void divide(V& v, const T& s) { std::for_each(v.begin(), v.end(), [&s](atbegin_t<V>& x) { x /= s; }); }

/// scalar addition v += s
template<typename V, typename T>
inline void add(V& v, const T& s) { std::for_each(v.begin(), v.end(), [&s](atbegin_t<V>& x) { x += s; }); }

/// element-wise sum
template<typename V>
inline atbegin_t<V> sum(const V& v) { return std::accumulate(v.begin(), v.end(), atbegin_t<V>{}); }

/// Vector sum a+b
template<typename T, typename U>
T vsum(const T& a, const U& b) {
    T d = a;
    auto itb = b.begin();
    for(auto& x: d) x += *(itb++);
    return d;
}

/// Vector sum a + k*b
template<typename T, typename U, typename V>
T vsum(const T& a, const U& k, const V& b) {
    T d = a;
    auto itb = b.begin();
    for(auto& x: d) x += (*(itb++))*k;
    return d;
}

/// Vector difference a-b
template<typename T, typename U>
T vdiff(const T& a, const U& b) {
    T d = a;
    auto itb = b.begin();
    for(auto& x: d) x -= *(itb++);
    return d;
}

/// Vector dot product; for mixed vector types, convert to contents of first
template<typename T, typename U>
inline array_contents_t<T> dot(const T& a,  const U& b) {
    array_contents_t<T> d{};
    size_t i = 0;
    for(auto v: a) d += v * array_contents_t<T>(b[i++]);
    return d;
}

/// arithmetic types mag^2
template<typename T, typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr>
inline T mag2(const T& x) { return x*x; }

/// vector magnitude^2
template<typename V, typename std::enable_if<!std::is_arithmetic<V>::value>::type* = nullptr>
inline atbegin_t<V> mag2(const V& v) {
    typedef atbegin_t<V> T;
    return std::accumulate(v.begin(), v.end(), T{}, [](const T& a, const T& b) { return a + b*b; });
}

/// vector magnitude
template<typename T>
inline array_contents_t<T> mag(const T& v) { return sqrt(mag2(v)); }

/// vector cross product
template<typename T>
inline void cross(const T& a, const T& b, T& c) {
    static_assert(std::tuple_size<T>::value == 3, "Cross-product only defined for 3-vector");
    c[0] = a[2]*b[1] - a[1]*b[2];
    c[1] = a[2]*b[0] - a[0]*b[2];
    c[2] = a[0]*b[1] - a[1]*b[0];
}

/// vector triple product
template<typename T>
inline array_contents_t<T> triple_prod(const T& a,  const T& b, const T& c) {
    static_assert(std::tuple_size<T>::value == 3, "Triple-product only defined for 3-vector");
    return a[0]*b[1]*c[2] + a[2]*b[0]*c[1] + a[1]*b[2]*c[0] - a[2]*b[1]*c[0] - a[1]*b[0]*c[2] - a[0]*b[2]*c[1];
}

/// normalize to unit vector; return original length
template<typename T>
inline atbegin_t<T> makeunit(T& v) {
    auto d = mag(v);
    divide(v, d);
    return d;
}

#endif
