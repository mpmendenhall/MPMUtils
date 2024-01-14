/// @file ThreadDataSerializer.hh FIFO processing queue for serializing data from multiple threads
// -- Michael P. Mendenhall, 2019

#ifndef THREADDATASERIALIZER_HH
#define THREADDATASERIALIZER_HH

#include "Threadworker.hh"
#include <vector>
using std::vector;

/// FIFO processing queue for collecting/serializing input from multiple threads
template<typename T>
class ThreadDataSerializer: public Threadworker {
public:
    /// Data type being serialized
    typedef T data_t;

    /// Constructor
    ThreadDataSerializer() { }
    /// Destructor
    virtual ~ThreadDataSerializer() { clear_pool(); }

    /// Thread-safe get allocated object space, or nullptr if priority-0 allocation rejected
    //  likely called from multiple input threads
    virtual T* get_allocated(int priority = 0) {
        lock_guard<mutex> plk(pmutex);
        if(!pool.size()) {
            if(!priority && maxAllocate && nAllocated >= maxAllocate) return nullptr;
            ++nAllocated;
            return allocate_new();
        }
        auto obj = pool.back();
        pool.pop_back();
        return obj;
    }

    /// Thread-safe return object for processing; pass nullptr to end processing
    //  likely called from multiple input threads
    void return_allocated(T* obj) {
        lock_guard<mutex> lk(inputMut);
        queue.push_back(obj);       // add item to queue
        inputReady.notify_one();    // notify that queue item is ready for processing
    }

    /// Thread-safe toggle of halt flag
    void set_halt(bool h) {
        lock_guard<mutex> lk(inputMut);
        halt = h;
        if(halt) inputReady.notify_one(); // notification to catch halt
    }

    /// receive and process items as they are placed in queue; terminate on queued nullptr or "halt" flag
    //  run directly in main thread, or spawn in separate thread by launch_mythread
    void threadjob() override {
        vector<T*> v;
        bool qbreak = false; // encountered nullptr break in queue?
        while(!halt && !qbreak) {
            { // scope for queue lock
                unique_lock<mutex> lk(inputMut); // acquire unique_lock on queue in this scope
                inputReady.wait(lk, [this]{return queue.size() || halt;}); // unlock until notified
                if(!halt) qbreak = extract_to_break(v);
            }

            process_items(v);
        }
        if(qbreak) end_of_processing();
    }

    size_t maxAllocate = 0;         ///< max events to allocate; 0 for unlimited

protected:

    /// process item received in queue
    /// return 'true' to return_pool now, or 'false' if we will manually return_pool later
    virtual bool process_item(T& /*obj*/) { return true; }
    /// run at termination of processing loop (within process_queued()'s thread)
    virtual void end_of_processing() { }
    /// clear re-usable returned objects; must be thread-safe
    virtual void reset_allocated(T& /*obj*/) { }

    /// creation of new allocation objects; must be thread-safe
    virtual T* allocate_new() { return new T; }
    /// final deallocation of pool objects, probably in destructor
    virtual void deallocate(T* obj) { delete obj; }
    /// thread-safe return of one item to pool
    void return_pool(T* obj) {
        reset_allocated(*obj);
        lock_guard<mutex> plk(pmutex);
        pool.push_back(obj);
    }
    /// deallocate all pooled objects, probably in destructor
    void clear_pool() {
        lock_guard<mutex> plk(pmutex);
        for(auto p: pool) deallocate(p);
        pool.clear();
    }

    /// extract items from queue up to nullptr break
    bool extract_to_break(vector<T*>& v) {
        auto itq = queue.begin();
        for(; itq != queue.end(); itq++) {
            if(!*itq) break;
            v.push_back(*itq);
        }
        bool qbreak = itq != queue.end();
        // shift remaining items to start of queue
        auto itq2 = queue.begin();
        for(; itq < queue.end(); itq++) *(itq2++) = *itq;
        queue.resize(itq2 - queue.begin());
        return qbreak;
    }

    /// process multiple items
    void process_items(vector<T*>& v) {
        auto itv = v.begin();
        for(auto p: v) if(process_item(*p)) *(itv++) = p;

        // bulk return to pool
        if(itv != v.begin()) {
            lock_guard<mutex> plk(pmutex);
            for(auto it = v.begin(); it != itv; ++it) {
                if(!*it) continue;
                reset_allocated(**it);
                pool.push_back(*it);
            }
        }
        v.clear();
    }

    /// flush queued items up to next break
    void flush_queued_to_break() {
        vector<T*> v;
        { // scope for queue lock
            lock_guard<mutex> lk(inputMut);
            extract_to_break(v);
        }
        process_items(v);
    }

    /// discard queued items
    void discard_queued() {
        lock_guard<mutex> lk(inputMut);
        lock_guard<mutex> plk(pmutex);
        for(auto i: queue) if(i) { reset_allocated(*i); pool.push_back(i); }
        queue.clear();
    }

    vector<T*> pool;        ///< re-usable allocated objects pool
    mutex pmutex;           ///< mutex for pool access
    vector<T*> queue;       ///< items received in processing queue --- lock with inputMut

    size_t nAllocated = 0;  ///< number of items allocated
    bool halt = false;      ///< processing halt flag (needs qmutex)
};

#endif

