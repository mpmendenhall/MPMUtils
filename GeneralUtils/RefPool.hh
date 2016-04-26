/// \file RefPool.hh Base class for "pool" of re-usable, reference-counted objects
// This file was produced under the employ of the United States Government,
// and is consequently in the PUBLIC DOMAIN, free from all provisions of
// US Copyright Law (per USC Title 17, Section 105).
//
// -- Michael P. Mendenhall, 2016

#ifndef REFPOOL_HH
#define REFPOOL_HH

#include "RefCounter.hh"
#include <vector>
using std::vector;
#include <set>
using std::set;

class RefPool;

/// Reference-counted item which returns itself to a pool for re-use
class RefPoolItem: public RefCounter {
friend class RefPool;
public:
    /// Constructor
    RefPoolItem(RefPool* P, bool rtn = false): RefCounter(rtn), myPool(P) { }
    /// Decrement reference count
    void release() override;
    /// Re-set contents for re-use
    virtual void clear() { }
    
protected:
    RefPool* myPool;     ///< pool to which this object belongs
};

/// Pool of re-usable reference-counted items
class RefPool {
friend class RefPoolItem;
public:    
    /// Destructor
    virtual ~RefPool();
    /// Check (cleared, released) item out from pool
    virtual RefPoolItem* checkout();
protected:
    /// allocate new item (subclass correct type; assign to this pool)
    virtual RefPoolItem* newItem() = 0;
    /// return to pool
    void returnItem(RefPoolItem* i) { checkedout.erase(i); items.push_back(i); }
    
    vector<RefPoolItem*> items;         ///< pool items awaiting re-use
    set<RefPoolItem*> checkedout;       ///< checked-out items
};

#endif
