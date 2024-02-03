/// @file ThreadBufferSink.hh FIFO Buffered input to sink running in independent thread
// Michael P. Mendenhall, LLNL 2021

#ifndef THREADBUFFERSINK_HH
#define THREADBUFFERSINK_HH

#include "DataSink.hh"
#include "PingpongBufferWorker.hh"
#include <unistd.h>

/// Buffered input to sink running in independent thread
template<typename T>
class ThreadBufferSink: public DataLink<T,T>, public PingpongBufferWorker<T>, public Configurable {
public:
    using DataLink<T,T>::nextSink;
    typedef PingpongBufferWorker<T> PBW;
    using PBW::verbose;
    using PBW::_datq;

    /// Configuration constructor
    explicit ThreadBufferSink(const Setting& S): Configurable(S) {
        Cfg.lookupValue("verbose", verbose, "threading debug verbosity level");
        if(Cfg.show_exists("next", "ThreadBufferSink downstream analysis chain")) this->createOutput(Cfg["next"]);
    }

    /// receive item to queue
    void push(T& o) override { add_item(o); }

    /// buffered signal
    struct bufsig_t {
        size_t i;                   ///< datastream position
        datastream_signal_t sig;    ///< signal
    };

    /// handle signals
    void signal(datastream_signal_t sig) override {
        if(sig == DATASTREAM_INIT) PBW::launch_mythread();
        {
            lock_guard<mutex> l(PBW::inputMut);
            sigq.push_back({PBW::datq.size(), sig});
            PBW::inputReady.notify_one();
            sched_yield();
        }
        if(sig >= DATASTREAM_END) PBW::finish_mythread(true);
    }

protected:
    vector<bufsig_t> sigq;      ///< signals input FIFO
    vector<bufsig_t> _sigq;     ///< ping-pong for sigq

    /// swap input/output buffers
    void pingpong() override {
        PBW::pingpong();
        std::swap(sigq, _sigq);
    }

    /// process output buffers
    void processout() override {
        PBW::processout();

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

        _sigq.clear();
    }
};

#endif
