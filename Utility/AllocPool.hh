/// \file AllocPool.hh Pool of re-usable allocated objects
// -- Michael P. Mendenhall 2018

#ifndef ALLOCPOOL_HH
#define ALLOCPOOL_HH

#include <vector>
#include <mutex>
#include <cassert>
using std::vector;


/// Pool of re-usable allocated objects
template<class T>
class AllocPool {
public:
    /// Detructor
    virtual ~AllocPool() { for(auto p: pool) delete p; }
    /// get allocated item
    T* get() {
        if(!pool.size()) { nAlloc++; return new T; }
        auto i = pool.back();
        pool.pop_back();
        return i;
    }
    /// Return allocated item
    void put(T* p) {
        p->clear();
        if(pool.size() < maxPool) pool.push_back(p);
        else delete p;
    }
protected:
    size_t nAlloc = 0;      ///< total number of items allocated
    size_t maxPool = 4096;  ///< maximum pool size before deletion
    vector<T*> pool;        ///< allocated object pool
};

/// Thread-safe AllocPool
template<class T>
class LockedAllocPool {
public:
    /// Detructor
    virtual ~LockedAllocPool() { for(auto p: pool) delete p; }
    /// get allocated item
    T* get() {
        T* i = nullptr;
        {
            std::unique_lock<std::mutex> lk(poolLock);
            if(!pool.size()) { nAlloc++; return new T; }
            i = pool.back();
            pool.pop_back();
        }
        return i;
    }
    /// Return allocated item
    void put(T* p) {
        p->clear();
        std::unique_lock<std::mutex> lk(poolLock);
        pool.push_back(p);
    }
protected:
    size_t nAlloc = 0;      ///< total number of items allocated
    vector<T*> pool;        ///< allocated object pool
    std::mutex poolLock;    ///< lock on pool
};

#endif
