/// \file OrderingQueue.hh Sort slightly-out-of-order events into proper order
// -- Michael P. Mendenhall, LLNL 2019

#ifndef ORDERINGQUEUE_HH
#define ORDERINGQUEUE_HH

#include "SinkUser.hh"
#include "deref_if_ptr.hh"
#include "SFINAEFuncs.hh" // for dispObj

#include <queue>
using std::priority_queue;
#include <cassert>
#include <limits>
#include <cmath> // for std::isfinite
#include <csignal> // for SIGINT breakpoint

/// Sort slightly-out-of-order items into proper order
template<class T, typename _ordering_t = typename std::remove_pointer<T>::type::ordering_t>
class OrderingQueue: public DataLink<const T, T> {
public:
    /// input type
    using typename DataSink<const T>::sink_t;
    /// ordering operator
    typedef _ordering_t ordering_t;
    /// output type
    using typename SinkUser<T>::output_t;
    /// mutable data
    using typename DataSink<sink_t>::mutsink_t;
    /// queue type
    typedef priority_queue<mutsink_t, vector<mutsink_t>, reverse_ordering_deref<mutsink_t, ordering_t>> PQ_t;

    static constexpr ordering_t order_max = std::numeric_limits<ordering_t>::max();

    /// Constructor
    explicit OrderingQueue(DataSink<T>* S = nullptr, ordering_t _dt = order_max): dt(_dt) { this->nextSink = S; }
    /// Destructor --- please leave cleared!
    ~OrderingQueue() {
        if(!PQ.empty()) {
            fprintf(stderr, "\n*** WARNING:  OrderingQueue destructed with %zu elements remaining:\n", PQ.size());
            while(!PQ.empty()) {
                dispObj(PQ.top());
                PQ.pop();
            }
            std::raise(SIGINT);
        }
    }

    /// number of items in queue
    size_t size() const { return PQ.size(); }

    /// get ordering parameter of object
    template<typename U>
    static inline ordering_t order(U o) { return ordering_t(deref_if_ptr(o)); }

    /// clear remaining objects through window
    void signal(datastream_signal_t sig) override {
        if(sig >= DATASTREAM_FLUSH) {
            while(!PQ.empty()) {
                auto o = PQ.top();
                processOrdered(o);
                PQ.pop();
            }
            t0 = -order_max;
        }
        if(this->nextSink) this->nextSink->signal(sig);
    }

    /// flush events up to specified point
    void flushTo(ordering_t t) {
        t0 = t;
        while(!PQ.empty()) {
            auto o = PQ.top();
            if(order(o) >= t0) break;
            PQ.pop();
            processOrdered(o);
        }
    }

    /// add new item to sorted queue, with auto-flush
    void push(sink_t& o) override { push(o, true); }

    /// add new item to sorted queue; optionally flush
    void push(sink_t& o, bool doFlush) {

        ordering_t t = order(o);

        if(!std::isfinite(t)) {
            printf("Passing through un-orderable object!\n");
            dispObj(o);
            if(skip_disordered) return;
            output_t oo = o;
            processOrdered(oo);
            return;
        }

        if(t < t0) {
            if(!--ndis) {
                printf("Warning: out-of-order queue event at %g < %g (%g)!\n",
                       double(t), double(t0), double(t0-t));
                dispObj(o);
                ndis = warn_ndis;
                throw std::logic_error("OrderingQueue input before minimum");
            }

            if(skip_disordered) return;
            output_t oo = o;
            processOrdered(oo);
            return;
        }

        PQ.push(o);

        if(doFlush) flushTo(t-dt);
    }

    ordering_t t0 = -order_max; ///< flush boundary
    ordering_t dt;              ///< flush ordered queue more than this far before highest item
    int warn_ndis = 1;          ///< frequency to print disordered-event warning
    int ndis = 1;               ///< number disordered since last warning
    bool skip_disordered = true;///< skip over disordered events

protected:
    PQ_t PQ;

    /// pass down chain
    virtual void processOrdered(output_t& o) { if(this->nextSink) this->nextSink->push(o); }
};

#endif
