/// \file RefCounter.hh \brief Simple base class for reference-counted objects
#ifndef REFCOUNTER_HH
#define REFCOUNTER_HH

#include <cassert>

/// Reference-counted object base class
class RefCounter {
public:
    /// Constructor
    RefCounter(bool rtn = false) { if(rtn) retain(); }
    /// Destructor
    virtual ~RefCounter() { assert(!nrefs); }
    /// Increment reference count
    virtual void retain() { nrefs++; }
    /// Decrement reference count
    virtual void release() { assert(nrefs); nrefs--; if(!nrefs) delete(this); }
    
protected:
    unsigned int nrefs = 0; ///< reference count
};

#endif
