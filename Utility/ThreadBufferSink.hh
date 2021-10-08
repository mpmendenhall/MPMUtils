/// \file ThreadBufferSink.hh Buffered input to sink running in independent thread
// Michael P. Mendenhall, LLNL 2021

#ifndef THREADBUFFERSINK_HH
#define THREADBUFFERSINK_HH

#include "DataSink.hh"
#include "Threadworker.hh"
#include <unistd.h>

/// Buffered input to sink running in independent thread
template<typename T>
class ThreadBufferSink: public DataLink<T,T>, public Threadworker {
public:
    typedef typename std::remove_const<T>::type Tmut_t;
    using DataLink<T,T>::nextSink;

    /// Constructor
    ThreadBufferSink(DataSink<T>* s = nullptr) { nextSink = s; }

    /// receive item to queue (or pass through if queue not running)
    void push(T& o) override {
        if(!is_launched) {
            if(nextSink) nextSink->push(o);
            return;
        }
        lock_guard<mutex> l(inputMut);
        datq.push_back(o);
        inputReady.notify_one();
        sched_yield();
    }

    /// thread to pull from queue and push downstream
    void threadjob() override {
        do {
            {
                unique_lock<mutex> lk(inputMut);  // acquire unique_lock on queue in this scope
                inputReady.wait(lk, [this]{ return all_done || !datq.empty(); });  // unlock until notified
                std::swap(datq, datq2);
            }
            if(nextSink) {
                for(auto& o: datq2) {
                    nextSink->push(o);
                    sched_yield();
                }
            }
            datq2.clear();
        } while(!all_done);

        flush();
    }

    /// handle signals, including flush
    void signal(datastream_signal_t sig) override {
        bool _is_launched = is_launched;
        if(is_launched) finish_mythread();
        else if(sig >= DATASTREAM_FLUSH) flush();
        if(nextSink) nextSink->signal(sig);
        if(_is_launched) launch_mythread();
    }

protected:
    /// flush all remaining queued data
    void flush() {
        if(nextSink) {
            for(auto& o: datq) {
                nextSink->push(o);
                sched_yield();
            }
        }
        datq.clear();
    }

    vector<Tmut_t> datq;    ///< input FIFO
    vector<Tmut_t> datq2;   ///< swappable copy
};

#endif
