/// \file ThreadBufferSink.hh FIFO Buffered input to sink running in independent thread
// Michael P. Mendenhall, LLNL 2021

#ifndef THREADBUFFERSINK_HH
#define THREADBUFFERSINK_HH

#include "DataSink.hh"
#include "Threadworker.hh"
#include <unistd.h>

/// Typeless base
class _ThreadBufferSink: public Configurable, public XMLProvider,
virtual public _DataSink, virtual public _SinkUser {

};

/// Buffered input to sink running in independent thread
template<typename T>
class ThreadBufferSink: public DataLink<T,T>, public Threadworker {
public:
    typedef typename std::remove_const<T>::type Tmut_t;
    using DataLink<T,T>::nextSink;

    /// Constructor
    ThreadBufferSink(DataSink<T>* s = nullptr) { nextSink = s; }

    /// receive item to queue
    void push(T& o) override {
        lock_guard<mutex> l(inputMut);
        datq.push_back(o);
        inputReady.notify_one();
        sched_yield();
    }

    /// thread to pull from queue and push downstream
    void threadjob() override {
        vector<Tmut_t> datq2;

        while(true) {
            check_pause();

            { // get input items ready to pass on
                unique_lock<mutex> lk(inputMut);    // acquire unique_lock on queue in this scope
                if(runstat == STOP_REQUESTED) break;
                inputReady.wait(lk);                // unlock; wait; re-lock when notified
                std::swap(datq, datq2);
            }

            // push(...) while continuing to receive without blocking
            if(nextSink) {
                for(auto& o: datq2) {
                    nextSink->push(o);
                    sched_yield();
                }
            }
            datq2.clear();
        }
    }

    /// handle signals, including flush
    void signal(datastream_signal_t sig) override {
        bool _is_launched = checkRunning();
        if(_is_launched) pause();
        if(sig >= DATASTREAM_FLUSH) {
            lock_guard<mutex> l(inputMut);
            if(nextSink) for(auto& o: datq) nextSink->push(o);
            datq.clear();
        }
        if(nextSink) nextSink->signal(sig);
        if(_is_launched) unpause();
    }

protected:
    vector<Tmut_t> datq;    ///< input FIFO
};

#endif
