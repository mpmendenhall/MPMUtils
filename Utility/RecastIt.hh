/// \file RecastIt.hh re-casting iterator wrapper
// Michael P. Mendenhall, 2018

#ifndef RECASTIT_HH
#define RECASTIT_HH

#include <iterator>

/// re-casting iterator wrapper
template<class it0, class T>
class recastIt: public std::iterator<std::forward_iterator_tag,T*> {
public:
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
