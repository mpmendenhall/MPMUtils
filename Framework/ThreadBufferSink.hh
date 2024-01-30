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
        lock_guard<mutex> l(inputMut);
        datq.push_back(o);
        inputReady.notify_one();
        sched_yield();
    }

    /// buffered signal
    struct bufsig_t {
        size_t i;                   ///< datastream position
        datastream_signal_t sig;    ///< signal
    };

    /// thread to pull from queue and push downstream
    void threadjob() override {
        while(true) {
            check_pause();

            { // get input items ready to pass on
                unique_lock<mutex> lk(inputMut);    // acquire unique_lock on queue in this scope
                if(runstat == STOP_REQUESTED) {
                    if(verbose > 3) printf(TERMFG_YELLOW "  Threadworker [%i] got stop command." TERMSGR_RESET "\n", worker_id);
                    break;
                } else if(verbose > 4) printf(TERMFG_BLUE "  Threadworker [%i] awaiting new input." TERMSGR_RESET "\n", worker_id);
                inputReady.wait(lk);                // unlock; wait; re-lock when notified
                pingpong();
            }

            // push(...) while continuing to receive without blocking
            processout();
        }

        if(verbose > 3)
            printf(TERMFG_BLUE "  Threadworker [%i] done (max buffered: %zu)." TERMSGR_RESET "\n", worker_id, most_buffered);
        most_buffered = 0;
    }

    /// handle signals
    void signal(datastream_signal_t sig) override {
        if(sig == DATASTREAM_INIT) launch_mythread();
        {
            lock_guard<mutex> l(inputMut);
            sigq.push_back({datq.size(), sig});
            inputReady.notify_one();
            sched_yield();
        }
        if(sig >= DATASTREAM_END) {
            finish_mythread();
            pingpong();
            processout();
        }
    }

protected:
    vector<Tmut_t> datq;        ///< input FIFO
    vector<Tmut_t> _datq;       ///< ping-pong for datq
    vector<bufsig_t> sigq;      ///< signals input FIFO
    vector<bufsig_t> _sigq;     ///< ping-pong for sigq

    /// swap input/output buffers
    void pingpong() {
        if(_datq.size() || _sigq.size()) throw std::logic_error("output buffer uncleared");
        std::swap(datq, _datq);
        std::swap(sigq, _sigq);
        most_buffered = std::max(most_buffered, _datq.size());
    }

    /// process output buffers
    void processout() {
        if(verbose > 4)
            printf(TERMFG_BLUE "  Threadworker [%i] has %zu buffered + %zu signals to process." TERMSGR_RESET "\n",
                worker_id, _datq.size(), _sigq.size());
        if(nextSink) {
            size_t i = 0;
            auto sit = _sigq.begin();
            while(sit != _sigq.end() || i < _datq.size())  {
                while(sit != _sigq.end() && sit->i <= i) {
                    DataLink<T,T>::signal(sit->sig);
                    ++sit;
                }
                if(i < _datq.size()) nextSink->push(_datq[i]);
                sched_yield();
                ++i;
            }
        }
        _datq.clear();
        _sigq.clear();
    }

    size_t most_buffered = 0;   ///< record most items buffered
};

#endif
