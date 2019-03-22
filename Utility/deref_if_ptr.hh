/// \file deref_if_ptr.hh Template utility to return T&, T* -> T&
// Michael P. Mendenhall, 2019

#ifndef DEREF_IF_PTR_HH
#define DEREF_IF_PTR_HH

#include <type_traits>

/// For templates on pointer or object
template<typename T>
inline T& deref_if_ptr(T& obj) { return obj; }

/// For templates on pointer or object
template<typename T>
inline T& deref_if_ptr(T* obj) { return *obj; }

/// For templates on pointer or object
template<typename T>
inline const T& deref_if_ptr(const T& obj) { return obj; }

/// For templates on pointer or object
template<typename T>
inline const T& deref_if_ptr(const T* obj) { return *obj; }

/// forward-ordering helper
template<typename T0, typename ordering_t>
struct forward_ordering_deref {
    /// forward ordering comparison
    bool operator()(const T0& a, const T0& b) const { return ordering_t(deref_if_ptr(a)) < ordering_t(deref_if_ptr(b)); }
};

/// reverse-ordering helper
template<typename T0, typename ordering_t>
struct reverse_ordering_deref {
    /// reverse ordering comparison
    bool operator()(const T0& a, const T0& b) const { return ordering_t(deref_if_ptr(b)) < ordering_t(deref_if_ptr(a)); }
};

#endif
