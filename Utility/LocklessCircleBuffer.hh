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
    LocklessCircleBuffer(size_t n = 1024) { allocate(n); }

    /// change buffer size
    void allocate(size_t n) {
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
    T* get_writepoint(double t_s, bool fail_OK = true) {
        if(writept) throw std::logic_error("Unfinished write in progress");

        if(!t_s) { // immediate return without timeout
            // does write point have a pending read?
            if(!ready[write_idx]) writept = &buf[write_idx];
        } else {
            auto t = std::chrono::steady_clock::now() + std::chrono::milliseconds(int(1e3*t_s));
            do {
                // does write point have a pending read?
                if(ready[write_idx]) { usleep(1000); continue; }
                writept = &buf[write_idx];
            } while(!writept && std::chrono::steady_clock::now() < t);
        }

        if(!writept) {
            if(!fail_OK) throw std::logic_error("Timeout waiting for write point");
            ++n_write_fails;
        }
        return writept;
    }

    /// call after completing access to write point, appending to processing queue
    void finish_write() {
        if(!writept) throw std::logic_error("No write in progress");
        __sync_synchronize();
        ready[write_idx] = true; // mark as written
        write_idx = (write_idx + 1) % buf.size();
        writept = nullptr;
        if(!checkRunning()) flush();
        else inputReady.notify_one(); // notify new read available
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
        process_item();
        auto r0 = read_idx;
        read_idx = (read_idx + 1)%buf.size();
        ready[r0] = false;
        return true;
    }

    /// consume all next available items
    size_t flush() {
        int nread = 0;
        while(read_one()) ++nread;
        return nread;
    }

    /// count number of buffered items... not guaranteed correct
    size_t n_buffered() const {
        auto iw = write_idx;
        auto ir = read_idx;
        if(iw < ir) iw += buf.size();
        return iw==ir? (ready[ir]? buf.size() : 0) : iw - ir - 1;
    }

    /// wait for buffer clear to fraction, with timeout (s) and optional error
    void wait_buffer(double timeout, double frac = 0.2, double fail_OK = true) const {
        size_t targ = frac * buf.size();
        auto t = std::chrono::steady_clock::now() + std::chrono::milliseconds(int(1e3*timeout));
        while(std::chrono::steady_clock::now() < t) {
            if(n_buffered() <= targ) return;
            usleep(1000);
        }
        if(!fail_OK) throw std::logic_error("Timeout waiting for buffer clear");
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
    vector<int> ready;      ///< ready-to-read indicator for each buffer item; int to encourage atomic updating
    T* writept = nullptr;   ///< current item being modified
    T current;              ///< current item to process reading
    size_t write_idx = 0;   ///< write point marker
    size_t read_idx = 0;    ///< read point marker
};

#endif
