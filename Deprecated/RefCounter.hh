/// \file RefCounter.hh Simple base class for reference-counted objects

// This file was produced under the employ of the United States Government,
// and is consequently in the PUBLIC DOMAIN, free from all provisions of
// US Copyright Law (per USC Title 17, Section 105).
//
// -- Michael P. Mendenhall, 2015

#ifndef REFCOUNTER_HH
#define REFCOUNTER_HH

#include <cassert>

/// Reference-counted object base class
class RefCounter {
public:
    /// Constructor
    explicit RefCounter(bool rtn = false) { if(rtn) retain(); }
    /// Destructor
    virtual ~RefCounter() { assert(!nrefs); }
    /// Increment reference count
    inline void retain() { ++nrefs; }
    /// Decrement reference count
    virtual void release() { assert(nrefs); if(!--nrefs) delete this; }

protected:
    unsigned int nrefs = 0; ///< reference count
};

#endif
