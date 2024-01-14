/// @file LocklessCircleBuffer.hh Circular buffer for passing output from time-sensitive thread

#ifndef LOCKLESSCIRCULARBUFFER_HH
#define LOCKLESSCIRCULARBUFFER_HH

#include "Threadworker.hh"

#include <unistd.h>     // for usleep
#include <chrono>       // for timeouts

/// Circular buffer base class
template<typename T>
class LocklessCircleBuffer: public Threadworker {
public:
    /// Constructor
    LocklessCircleBuffer(size_t n = 1024): buf(n), ready(n) { }

    /// change buffer size
    virtual void allocate(size_t n) {
        buf.clear();
        ready.clear();
        buf.resize(n);
        ready.resize(n);
    }

    /// get pointer to next buffer space; nullptr if unavailable
    T* get_writepoint() {
        if(writept) throw std::logic_error("Unfinished write in progress");
        if(ready[write_idx]) { ++n_write_fails; return nullptr; }
        return writept = &buf[write_idx];
    }

    /// get pointer to next buffer space, with timeout in s; nullptr if unavailable
    T* get_writepoint(double t_s) {
        if(writept) throw std::logic_error("Unfinished write in progress");

        if(!t_s) {
            if(!ready[write_idx]) writept = &buf[write_idx];
        } else {
            auto t = std::chrono::steady_clock::now() + std::chrono::milliseconds(int(1e3*t_s));
            do {
                if(ready[write_idx]) { usleep(1000); continue; }
                writept = &buf[write_idx];
            } while(!writept && std::chrono::steady_clock::now() < t);
        }

        if(!writept) ++n_write_fails;
        return writept;
    }

    /// call after completing access to write point, appending to processing queue
    void finish_write() {
        if(!writept) throw std::logic_error("No write in progress");
        __sync_synchronize();
        ready[write_idx] = true;
        write_idx = (write_idx + 1)%buf.size();
        writept = nullptr;
        inputReady.notify_one();
    }

    /// write to next buffer space, failing if unavailable
    bool push_buffer(const T& a) {
        auto w = get_writepoint();
        if(!w) return false;
        *w = a;
        finish_write();
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

    /// task to be run in thread
    void threadjob() override {
        while(true) {
            check_pause();
            flush();
            unique_lock<mutex> lk(inputMut);  // acquire unique_lock on queue in this scope
            if(runstat == STOP_REQUESTED) break;
            inputReady.wait(lk);              // unlock until notified
        }
        flush();
    }

    /// processing on read item --- override me!
    virtual void process_item() = 0;

    size_t n_write_fails = 0;   ///< number of buffer-full write failures

protected:
    vector<T> buf;          ///< data buffer
    vector<int> ready;      ///< read indicator; int to encourage atomic updating
    T* writept = nullptr;   ///< current item being modified
    T current;              ///< current item to process
    size_t write_idx = 0;   ///< write point marker
    size_t read_idx = 0;    ///< read point marker
};

#endif
