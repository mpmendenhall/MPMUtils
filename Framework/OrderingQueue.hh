/// \file OrderingQueue.hh Sort slightly-out-of-order events into proper order
// -- Michael P. Mendenhall, LLNL 2019

#ifndef ORDERINGQUEUE_HH
#define ORDERINGQUEUE_HH

#include "DataSink.hh"
#include "deref_if_ptr.hh"
#include "SFINAEFuncs.hh" // for dispObj
#include <vector>
using std::vector;
#include <queue>
using std::priority_queue;
#include <stdio.h>  // for printf
#include <cassert>
#include <limits>
#include <cmath> // for std::isfinite
#include <stdexcept>
#include <stdio.h>
#include <csignal> // for SIGINT breakpoint

/// Sort slightly-out-of-order items into proper order
template<class T0, typename ordering_t = typename T0::ordering_t>
class OrderingQueue: public DataSink<T0>, public priority_queue<T0, vector<T0>, reverse_ordering_deref<T0, ordering_t>> {
public:

    /// parent type
    typedef priority_queue<T0, vector<T0>, reverse_ordering_deref<T0, ordering_t>> PQ;
    /// un-pointered class being ordered
    typedef typename std::remove_pointer<T0>::type T;

    static constexpr ordering_t order_max = std::numeric_limits<ordering_t>::max();

    /// Constructor
    OrderingQueue(ordering_t _dt = order_max): dt(_dt) { }
    /// Destructor --- please leave cleared!
    ~OrderingQueue() {
        if(!PQ::empty()) {
            fprintf(stderr, "\n*** WARNING:  OrderingQueue destructed with %zu elements remaining\n\n", PQ::size());
            std::raise(SIGINT);
            //throw std::logic_error("OrderingQueue destructed without flush");
        }
    }

    /// get ordering parameter of object
    template<typename U>
    static inline ordering_t order(U o) { return ordering_t(deref_if_ptr(o)); }

    /// clear remaining objects through window
    void signal(datastream_signal_t sig) override {
        if(sig < DATASTREAM_FLUSH) return;
        while(!PQ::empty()) {
            auto o = PQ::top();
            processOrdered(deref_if_ptr(o));
            PQ::pop();
        }
        t0 = -order_max;
    }

    /// flush events up to specified point
    void flushTo(ordering_t t) {
        t0 = t;
        while(!PQ::empty()) {
            auto o = PQ::top();
            if(order(o) >= t0) break;
            PQ::pop();
            processOrdered(deref_if_ptr(o));
        }
    }

    /// add new item to sorted queue, with auto-flush
    void push(const T0& o) override { push(o, true); }

    /// add new item to sorted queue; optionally flush
    void push(T0 o, bool doFlush) {

        ordering_t t = order(o);

        if(!std::isfinite(t)) {
            printf("Passing through un-orderable object!\n");
            dispObj(deref_if_ptr(o));
            if(skip_disordered) return;
            processOrdered(deref_if_ptr(o));
            return;
        }

        if(t < t0) {
            if(!--ndis) {
                printf("Warning: out-of-order queue event at %g < %g (%g)!\n",
                       double(t), double(t0), double(t0-t));
                dispObj(deref_if_ptr(o));
                ndis = warn_ndis;
                throw std::logic_error("OrderingQueue input before minimum");
            }

            if(skip_disordered) return;
            processOrdered(deref_if_ptr(o));
            return;
        }

        PQ::push(o);

        if(doFlush) flushTo(t-dt);
    }

    ordering_t t0 = -order_max; ///< flush boundary
    ordering_t dt;              ///< flush ordered queue more than this far before highest item
    int warn_ndis = 1;          ///< frequency to print disordered-event warning
    int ndis = 1;               ///< number disordered since last warning
    bool skip_disordered = true;///< skip over disordered events

protected:
    // -------------- subclass me! ------------------
    /// process outgoing ordered objects
    virtual void processOrdered(T&) = 0;
};

/// Ordering queue passing directly to DataSink
template<
    class TNext,
    typename T0 = typename TNext::sink_t,
    typename ordering_t = typename T0::ordering_t
>
class InlineOrderingQueue: public OrderingQueue<T0, ordering_t> {
public:
    /// parent class type
    typedef OrderingQueue<T0, ordering_t> super;
    /// inherit useful typedef
    typedef typename super::T T;

    /// Constructor, with output target
    InlineOrderingQueue(TNext* S = nullptr, ordering_t _dt = super::order_max): super(_dt) { nextSink = S; }

    TNext* nextSink = nullptr;

    /// clear remaining objects through window
    void signal(datastream_signal_t sig) override {
        super::signal(sig);
        if(this->nextSink) this->nextSink->signal(sig);
    }

protected:
    /// pass down chain
    void processOrdered(T& o) override { if(this->nextSink) this->nextSink->push(o); }
};

#endif
