/// @file RecastIt.hh re-casting iterator wrapper
// -- Michael P. Mendenhall, 2018

#ifndef RECASTIT_HH
#define RECASTIT_HH

#include <iterator>

/// re-casting iterator wrapper
template<class it0, class T>
class recastIt {
public:
    /// for STL iterator interface
    using iterator_category = std::forward_iterator_tag;
    /// for STL iterator interface
    using value_type = T*;
    /// for STL iterator interface
    using difference_type = std::ptrdiff_t;
    /// for STL iterator interface
    using pointer = value_type*;
    /// for STL iterator interface
    using reference = value_type&;

    /// Constructor
    recastIt(it0 it): I(it) { }

    /// increment
    recastIt& operator++() { ++I; return *this; }
    /// comparison
    bool operator==(const recastIt& rhs) const { return I == rhs.I; }
    /// inequality
    bool operator!=(const recastIt& rhs) const { return I != rhs.I; }
    /// dereference
    T*& operator*() { return (T*&)*I; }

    it0 I;  ///< underlying iterator
};

#endif
