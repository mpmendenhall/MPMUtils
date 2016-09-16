/// \file LocklessCircleBuffer.hh Circular buffer for passing output from time-sensitive thread

#ifndef LOCKLESSCIRCULARBUFFER_HH
#define LOCKLESSCIRCULARBUFFER_HH

#include <vector>
#include <unistd.h>     // for usleep
#include <pthread.h>

/// pthreads function for launching read process
template<class MyBufferType>
void* run_buffer_thread(void* p) {
    auto B = (MyBufferType*)p;
    while(!B->all_done) { B->flush(); usleep(B->sleep_us); }
    B->flush();
    return nullptr;
}


/// Circular buffer base class
template<typename T>
class LocklessCircleBuffer {
public:
    /// Constructor
    LocklessCircleBuffer(size_t n = 1024): buf(n), ready(n) { }
    /// Destructor
    virtual ~LocklessCircleBuffer() { }

    /// change buffer size
    virtual void allocate(size_t n) {
        buf.clear();
        ready.clear();
        buf.resize(n);
        ready.resize(n);
    }

    /// write to next buffer space, failing if unavailable
    bool write(const T& a) {
        if(ready[write_idx]) { n_write_fails++; return false; }
        buf[write_idx] = a;
        __sync_synchronize();
        ready[write_idx] = true;
        write_idx = (write_idx + 1)%buf.size();
        return true;
    }

    /// consume one next available item
    bool read_one() {
        if(!ready[read_idx]) return false;
        current = buf[read_idx];
        __sync_synchronize();
        ready[read_idx] = false;
        process_item();
        read_idx = (read_idx + 1)%buf.size();
        return true;
    }

    /// consume all next available items
    size_t flush() {
        int nread = 0;
        while(read_one()) nread++;
        return nread;
    }

    /// count number of buffered items... not guaranteed correct
    size_t n_buffered() const {
        size_t iw = write_idx;
        size_t ir = read_idx;
        if(ir > iw) iw += buf.size();
        return iw==ir? 0 : iw - ir - 1;
    }

    /// processing on read item --- override me!
    virtual void process_item() = 0;

    /// launch buffer thread
    virtual int launch_mythread() {
        is_launched = true;
        return pthread_create(&mythread, nullptr, run_buffer_thread<typename std::remove_reference<decltype(*this)>::type>, this);
    }

    /// finish clearing buffer thread
    virtual int finish_mythread() {
        all_done = true;
        int rc = pthread_join(mythread, NULL);
        is_launched = false;
        return rc;
    }

    size_t n_write_fails = 0;       ///< number of buffer-full write failures
    bool all_done = false;          ///< flag to indicate when all write operations complete
    useconds_t sleep_us = 50000;    ///< recommended sleep time between buffer clearing operations
    pthread_t mythread;             ///< identifier for buffer-clearing thread
    bool is_launched = false;       ///< marker for whether thread is launched

protected:
    std::vector<T> buf;             ///< data buffer
    std::vector<int> ready;         ///< read indicator; int to encourage atomic updating
    T current;                      ///< current item to process
    size_t write_idx = 0;           ///< write point marker
    size_t read_idx = 0;            ///< read point marker
};

#endif

