/// @file Collator.hh Combine ordered items received from multiple "push" sources
// -- Michael P. Mendenhall, 2019

#ifndef COLLATOR_HH
#define COLLATOR_HH

#include "_Collator.hh"
#include "SinkUser.hh"

#include "deref_if_ptr.hh"
#include "SFINAEFuncs.hh"

#include <queue>
#include <cassert>
#include <unistd.h>

/// Combine ordered items received from multiple "push" sources
template<typename T, typename _ordering_t = typename std::remove_pointer<T>::type::ordering_t>
class Collator: virtual public _Collator, public SinkUser<const T> {
public:
    typedef _ordering_t ordering_t;
    typedef typename std::remove_const<T>::type Tmut_t;
    using SinkUser<const T>::nextSink;

    /// Destructor: remember final flush.
    ~Collator() {
        if(!PQ.empty()) {
            printf("Warning: %zu items left in un-flushed collator queue\n", PQ.size());
            while(!PQ.empty()) {
                dispObj(PQ.top());
                PQ.pop();
            }
        }
        for(auto i: vInputs) delete i;
    }

    /// convenience input handle for this orderer
    class MOInput: public DataSink<T> {
    public:
        /// constructor
        explicit MOInput(Collator& _M, _SinkUser* s = nullptr):
        inSrc(s), n(_M.add_input()), M(&_M) {
            if(inSrc) {
                inSrc->_setNext(this);
                inSrc->setOwnsNext(false);
            }
        }
        /// DataSink push
        void push(T& o) override { M->push(n,o); }
        /// bulk push
        virtual void push(const vector<Tmut_t>& os) { M->push(n,os); }
        /// ignore signals
        void signal(datastream_signal_t) override { }

        _SinkUser* inSrc = nullptr; ///< input to this collator slot
        const size_t n;             ///< input enumeration

    protected:
        Collator* M;    ///< orderer
    };

    /// output all available collated items
    void process_ready() { while(!inputs_waiting && !PQ.empty()) pop(); }
    /// add item from enumerated input; output available collated
    void push(size_t nI, const T& o) { _push(nI, o); process_ready(); }
    /// bulk-add items
    void push(size_t nI, const vector<Tmut_t>& os) { _push(nI, os); process_ready(); }

    /// handle signals, including flush
    void signal(datastream_signal_t sig) override {
        lock_guard<mutex> lk(inputMut);
        if(sig >= DATASTREAM_FLUSH) {
            while(!PQ.empty()) {
                pop();
                sched_yield();
            }
        }
        if(nextSink) nextSink->signal(sig);
    }

    // --- multithreading support ---

    /// thread-safe push to queue for use in threadjob()
    void qpush(size_t nI, const T& o) {
        int myWait = input_n[nI].first;

        // time to clear buffer
        while(myWait > 32 && !inputs_waiting) {
            sched_yield();
            myWait = input_n[nI].first;
        }

        {
            lock_guard<mutex> l(inputMut);
            _push(nI, o);
            if(!inputs_waiting) inputReady.notify_one();
        }
        if(myWait > 32) usleep(1000*(myWait - 32));
        sched_yield();
    }

    /// thread-safe bulk-add items
    void qpush(size_t nI, const vector<Tmut_t>& os) {
        lock_guard<mutex> l(inputMut);
        _push(nI, os);
        inputReady.notify_one();
    }

    /// thread to pull from queue and push downstream
    void threadjob() override {
        do {
            vector<Tmut_t> v;
            {
                unique_lock<mutex> lk(inputMut);  // acquire unique_lock on queue in this scope
                inputReady.wait(lk, [this]{ return !inputs_waiting || runstat == STOP_REQUESTED; });  // unlock until notified
                while(!inputs_waiting && !PQ.empty()) {
                    auto& o = PQ.top();
                    if(!--input_n[o.first].first) ++inputs_waiting;
                    v.push_back(o.second);
                    PQ.pop();
                }
            }
            if(nextSink) {
                for(auto& o: v) {
                    nextSink->push(o);
                    sched_yield();
                }
            }

        } while(runstat != STOP_REQUESTED);

        signal(DATASTREAM_FLUSH);
    }

    /// convenience threaded input handle for this orderer
    class MOqInput: public MOInput {
    public:
        using MOInput::M;
        using MOInput::n;

        /// constructor
        explicit MOqInput(Collator& _M, _SinkUser* s = nullptr): MOInput(_M, s) { }
        /// DataSink push
        void push(T& o) override { M->qpush(n,o); }
        /// bulk push
        void push(const vector<Tmut_t>& os) override { M->qpush(n,os); }
    };

    /// connect SinkUser as input
    void connect_input(_SinkUser& s, int nreq = 0) override {
        vInputs.push_back(new MOqInput(*this, &s));
        change_required(vInputs.back()->n, nreq);
    }

    vector<MOInput*> vInputs;  ///< input adapters

protected:

    /// push to queue, update nwaiting
    void _push(size_t nI, const T& o) {
        if(!input_n.at(nI).first++) {
            --inputs_waiting;
            assert(inputs_waiting >= 0);
        }
        PQ.emplace(nI,o);
    }

    /// bulk-add items
    void _push(size_t nI, const vector<Tmut_t>& os) {
        int dn = os.size();
        if(!dn) return;
        auto& n = input_n.at(nI).first;
        if(n <= 0 && n + dn > 0) {
            --inputs_waiting;
            assert(inputs_waiting >= 0);
        }
        n += dn;
        for(auto& o: os) PQ.emplace(nI,o);
    }

    /// pop next element (and push to nextSink)
    void pop() {
        auto& o = PQ.top();
        if(!--input_n[o.first].first) ++inputs_waiting;
        if(nextSink) nextSink->push(const_cast<Tmut_t&>(o.second));
        PQ.pop();
    }

    /// one item from enumerated source
    struct iT: public std::pair<size_t,Tmut_t> {
        /// inherit constructors
        using std::pair<size_t,Tmut_t>::pair;
        /// reverse ordering operator
        bool operator<(const iT& b) const {
            return ordering_t(deref_if_ptr(b.second)) < ordering_t(deref_if_ptr(this->second));
        }
    };

    std::priority_queue<iT> PQ; ///< ordered inputs; lock on inputMut
};

#endif
