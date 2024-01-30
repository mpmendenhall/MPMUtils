/// @file ThreadBufferSink.hh FIFO Buffered input to sink running in independent thread
// Michael P. Mendenhall, LLNL 2021

#ifndef THREADBUFFERSINK_HH
#define THREADBUFFERSINK_HH

#include "DataSink.hh"
#include "Threadworker.hh"
#include <unistd.h>

/// Buffered input to sink running in independent thread
template<typename T>
class ThreadBufferSink: public DataLink<T,T>, public Threadworker, public Configurable {
public:
    typedef typename std::remove_const<T>::type Tmut_t;
    using DataLink<T,T>::nextSink;

    /// Configuration constructor
    explicit ThreadBufferSink(const Setting& S): Configurable(S) {
        Cfg.lookupValue("verbose", verbose, "threading debug verbosity level");
        if(Cfg.show_exists("next", "ThreadBufferSink downstream analysis chain")) this->createOutput(Cfg["next"]);
    }

    /// receive item to queue
    void push(T& o) override {
        if(!checkRunning()) launch_mythread();
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
                if(runstat == STOP_REQUESTED) {
                    if(verbose > 3) printf(TERMFG_YELLOW "  Threadworker [%i] got stop command." TERMSGR_RESET "\n", worker_id);
                    break;
                } else if(verbose > 4) printf(TERMFG_BLUE "  Threadworker [%i] awaiting new input." TERMSGR_RESET "\n", worker_id);
                inputReady.wait(lk);                // unlock; wait; re-lock when notified
                std::swap(datq, datq2);
            }

            most_buffered = std::max(most_buffered, datq2.size());
            if(verbose > 4) printf(TERMFG_BLUE "  Threadworker [%i] has %zu buffered to process." TERMSGR_RESET "\n",
                worker_id, datq2.size());

            // push(...) while continuing to receive without blocking
            if(nextSink) {
                for(auto& o: datq2) {
                    nextSink->push(o);
                    sched_yield();
                }
            }
            datq2.clear();
        }

        if(verbose > 3)
            printf(TERMFG_BLUE "  Threadworker [%i] done (max buffered: %zu)." TERMSGR_RESET "\n", worker_id, most_buffered);
        most_buffered = 0;
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
        DataLink<T,T>::signal(sig);
        if(_is_launched) {
            unpause();
            if(sig > DATASTREAM_FLUSH) finish_mythread();
        }
    }

protected:
    vector<Tmut_t> datq;        ///< input FIFO
    size_t most_buffered = 0;   ///< record most items buffered
};

#endif
