/// \file ThreadDataSerializer.hh FIFO processing queue for serializing data from multiple threads

#ifndef THREADDATASERIALIZER_HH
#define THREADDATASERIALIZER_HH

#include <vector>
#include <mutex>
#include <condition_variable>
#include <pthread.h>

/// pthreads function for launching processing loop
template<class MyTDSType>
void* queueprocess_thread(void* p) {
    ((MyTDSType*)p)->process_queued();
    return nullptr;
}

/// FIFO processing queue for collecting/serializing input from multiple threads
template<typename T>
class ThreadDataSerializer {
public:
    /// Constructor
    ThreadDataSerializer() { }
    /// Destructor
    virtual ~ThreadDataSerializer() { clear_pool(); }

    /// Thread-safe get allocated object space, or nullptr if allocation rejected
    virtual T* get_allocated(void* /*opts*/ = nullptr) {
        std::lock_guard<std::mutex> plk(pmutex);
        if(!pool.size()) return allocate_new();
        auto obj = pool.back();
        pool.pop_back();
        return obj;
    }

    /// Thread-safe return object for processing; pass nullptr to end processing
    void return_allocated(T* obj) {
        std::lock_guard<std::mutex> lk(qmutex);  // get lock on queue
        queue.push_back(obj);   // add item to queue
        qready.notify_one();    // notify that queue item is ready for processing
        // lock is released on exiting this scope
    }

    /// Thread-safe toggle of halt flag
    void set_halt(bool h) {
        std::lock_guard<std::mutex> lk(qmutex); // get lock on queue
        halt = h;
        if(halt) qready.notify_one();           // notification to catch halt
        // lock is released on exiting this scope
    }

    /// receive and process items as they are placed in queue; terminate on nullptr
    /// call directly or spawn in separate thread by launch_mythread
    void process_queued() {
        std::vector<T*> v;
        bool gothalt = false; // received halt flag?
        bool qbreak = false; // encountered nullptr break in queue?
        while(!gothalt && !qbreak) {
            { // scope for queue lock
                std::unique_lock<std::mutex> lk(qmutex); // acquire unique_lock on queue in this scope
                qready.wait(lk, [this]{return queue.size() || halt;}); // unlock until notified

                if(halt) gothalt = true;
                else {
                    // copy items from queue up to end or nullptr
                    auto itq = queue.begin();
                    for(; itq != queue.end(); itq++) {
                        if(!*itq) break;
                        v.push_back(*itq);
                    }
                    qbreak = itq != queue.end();
                    // shift remaining items to start of queue
                    auto itq2 = queue.begin();
                    for(; itq < queue.end(); itq++) *(itq2++) = *itq;
                    queue.resize(itq2 - queue.begin());
                }
            }

            // process queued items
            auto itv = v.begin();
            for(auto p: v) {
                if(process_item(*p)) *(itv++) = p;
            }

            // bulk return to pool
            if(itv != v.begin()) {
                std::lock_guard<std::mutex> plk(pmutex);
                for(auto it = v.begin(); it != itv; it++) {
                    if(!*it) continue;
                    reset_allocated(**it);
                    pool.push_back(*it);
                }
            }

            v.clear(); // re-usable in next round
        }
        if(qbreak) end_of_processing();
        is_launched = false;
    }

    /// launch buffer thread
    virtual int launch_mythread() {
        is_launched = true;
        return pthread_create(&mythread, nullptr, queueprocess_thread<typename std::remove_reference<decltype(*this)>::type>, this);
    }

    pthread_t mythread;             ///< identifier for queue processing thread
    bool is_launched = false;       ///< marker for whether thread is launched

protected:

    /// process item received in queue
    /// return 'true' to return_pool now, or 'false' if we will manually return_pool later
    virtual bool process_item(T& /*obj*/) { return true; }
    /// run at termination of processing loop
    virtual void end_of_processing() { }

    /// creation of new allocation objects
    virtual T* allocate_new() { return new T; }
    /// final deallocation of pool objects
    virtual void deallocate(T* obj) { delete obj; }
    /// clear re-usable returned objects
    virtual void reset_allocated(T& /*obj*/) { }
    /// thread-safe return of one item to pool
    void return_pool(T* obj) {
        reset_allocated(*obj);
        std::lock_guard<std::mutex> plk(pmutex);
        pool.push_back(obj);
    }
    /// deallocate all pooled objects
    void clear_pool() {
        std::lock_guard<std::mutex> plk(pmutex);
        for(auto p: pool) deallocate(p);
        pool.clear();
    }

    std::vector<T*> pool;       ///< re-usable allocated objects pool
    std::vector<T*> queue;      ///< items received in processing queue

    std::mutex pmutex;          ///< mutex for pool access
    std::mutex qmutex;          ///< mutex for queue access
    std::condition_variable qready; ///< wait for queue items or hault
    bool halt = false;          ///< processing halt flag (needs qmutex)
};

#endif

