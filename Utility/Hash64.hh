/// \file Hash64.hh Wrapper and convenience functions for 64-bit hashes (SipHash backend)
// -- Michael P. Mendenhall, LLNL 2019

#ifndef HASH64_HH
#define HASH64_HH

#include <cstddef>
#include <string>
using std::string;

// workaround for older gcc without std::is_trivially_copyable
#if __GNUG__ && __GNUC__ < 5
#define IS_TRIVIALLY_COPYABLE(T) __has_trivial_copy(T)
#else
#define IS_TRIVIALLY_COPYABLE(T) std::is_trivially_copyable<T>::value
#endif

/// 64-bit hash of binary data
size_t _hash64(const void* dat, size_t n);

/// 64-bit hash of string
inline size_t hash64(const string& s) { return _hash64(s.data(), s.size()); }

/// 64-bit hash of trivially-copyable object
template<typename T>
size_t hash64(const T& o) {
    static_assert(IS_TRIVIALLY_COPYABLE(T), "Object needs custom hash64 method");
    return _hash64(&o, sizeof(T));
}

/// combine hashes
size_t chash64(size_t a, size_t b);

/// 64-bit hash combining multiple arguments; h(a,b,c) = h(a, h(b,c))
template<typename T, typename... Args>
size_t hash64(const T& o, Args&&... a) { return chash64(hash64(o), hash64(std::forward<Args>(a)...)); }

#endif
