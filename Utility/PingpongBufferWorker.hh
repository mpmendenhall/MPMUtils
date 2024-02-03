/// @file PingpongBufferWorker.hh FIFO threaded ping-pong buffer processor
// Michael P. Mendenhall

#ifndef PINGPONGBUFFERWORKER_HH
#define PINGPONGBUFFERWORKER_HH

#include "Threadworker.hh"
#include "TermColor.hh"
#include <unistd.h>

/// Buffered input to sink running in independent thread
template<typename T>
class PingpongBufferWorker: public Threadworker {
public:
    typedef typename std::remove_const<T>::type Tmut_t;

    /// receive item to input buffer
    void add_item(T& o) {
        if(!checkRunning()) {
            _datq.push_back(o);
            processout();
            _datq.clear();
            return;
        }

        lock_guard<mutex> l(inputMut);
        datq.push_back(o);
        inputReady.notify_one();
        sched_yield();
    }

    /// thread to pull from queue and push downstream
    void threadjob() override {
        while(true) {
            check_pause();

            { // get input items ready to pass on
                unique_lock<mutex> lk(inputMut);    // acquire unique_lock on queue in this scope
                if(runstat == STOP_REQUESTED) {
                    if(verbose > 3) printf(TERMFG_YELLOW "  PingpongBufferWorker [%i] got stop command." TERMSGR_RESET "\n", worker_id);
                    break;
                } else if(verbose > 4) printf(TERMFG_BLUE "  PingpongBufferWorker [%i] awaiting new input." TERMSGR_RESET "\n", worker_id);
                inputReady.wait(lk);                // unlock; wait; re-lock when notified
                pingpong();
            }

            processout();
            _datq.clear();
        }

        if(verbose > 3)
            printf(TERMFG_BLUE "  PingpongBufferWorker [%i] done (max buffered: %zu)." TERMSGR_RESET "\n", worker_id, most_buffered);
        most_buffered = 0;
    }

    void finish_mythread(bool unlaunched_OK = false) override {
        Threadworker::finish_mythread(unlaunched_OK);
        pingpong();
        processout();
        _datq.clear();
    }

protected:
    vector<Tmut_t> datq;        ///< input FIFO
    vector<Tmut_t> _datq;       ///< ping-pong for datq
    size_t most_buffered = 0;   ///< record most items buffered

    /// swap input/output buffers
    virtual void pingpong() {
        if(_datq.size()) throw std::logic_error("output buffer uncleared");
        std::swap(datq, _datq);
        most_buffered = std::max(most_buffered, _datq.size());
    }

    /// process output buffer contents
    virtual void processout() {
        if(verbose > 4)
            printf(TERMFG_BLUE "  PingpongBufferWorker [%i] has %zu buffered." TERMSGR_RESET "\n", worker_id, _datq.size());
    }
};

#endif
