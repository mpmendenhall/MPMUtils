/// \file Collator.hh Combine ordered items received from multiple "push" sources
// -- Michael P. Mendenhall, 2019

#ifndef COLLATOR_HH
#define COLLATOR_HH

#include "DataSink.hh"
#include "deref_if_ptr.hh"

#include <cstddef> // for size_t on some systems
#include <queue>
#include <vector>
#include <utility>
#include <stdio.h>
#include <cassert>

/// Combine ordered items received from multiple "push" sources
template<class T, typename ordering_t = typename T::ordering_t>
class Collator: public SinkUser<T> {
public:
    typedef typename std::remove_const<T>::type Tmut_t;
    using SinkUser<T>::nextSink;

    /// polymorphic destructor: remember final flush.
    virtual ~Collator() {
        if(!PQ.empty()) printf("Warning: %zu items left in un-flushed collator queue\n", PQ.size());
    }

    /// add enumerated input
    size_t add_input(int nreq = 0) {
        ++inputs_waiting;
        size_t nI = input_n.size();
        input_n.emplace_back(0,0);
        if(nreq) change_required(nI,nreq);
        return nI;
    }

    /// change minimum number required from input
    void change_required(size_t nI, int i) {
        auto& n = input_n.at(nI).first;
        if(n <= 0 && n-i > 0) {
            if(inputs_waiting <= 0) throw std::logic_error("invalid inputs reduction");
            --inputs_waiting;
        }
        if(n > 0 && n-i <= 0) ++inputs_waiting;
        input_n[nI].second += i;
        n -= i;
    }
    /// get requirement threshold for input
    int get_required(size_t nI) const { return input_n.at(nI).second; }
    /// set minimum required from input
    void set_required(size_t nI, int i) { change_required(nI, i-get_required(nI)); }

    /// convenience input handle for this orderer
    class MOInput: public DataSink<T> {
    public:
        /// constructor
        MOInput(Collator& _M): M(_M), n(M.add_input()) { }
        /// DataSink push
        void push(T& o) override { M.push(n,o); }
        /// bulk push
        void push(std::vector<T>& os) { M.push(n,os); }

    protected:
        Collator& M;    ///< orderer
        size_t n;       ///< input enumeration
    };

    /// add item from enumerated input; output available collated
    void push(size_t nI, const T& o) {
        if(!input_n.at(nI).first++) {
            --inputs_waiting;
            assert(inputs_waiting >= 0);
        }
        PQ.emplace(nI,o);

        while(!inputs_waiting && !PQ.empty()) pop();
    }

    /// bulk-add items
    void push(size_t nI, const std::vector<T>& os) {
        int dn = os.size();
        if(!dn) return;
        auto& n = input_n.at(nI).first;
        if(n <= 0 && n + dn > 0) { --inputs_waiting; assert(inputs_waiting >= 0); }
        n += dn;
        for(auto& o: os) PQ.emplace(nI,o);

        while(!inputs_waiting && !PQ.empty()) pop();
    }

    /// flush all data
    virtual void signal(datastream_signal_t sig) {
        if(sig >= DATASTREAM_FLUSH) while(!PQ.empty()) pop();
        if(this->nextSink) this->nextSink->signal(sig);
    }

    /// clear all inputs
    virtual void reset() {
        signal(DATASTREAM_FLUSH);
        inputs_waiting = 0;
        input_n.clear();
    }

    /// get list of ``waiting'' inputs
    std::vector<size_t> get_waiting() const {
        std::vector<size_t> v;
        for(size_t i=0; i<input_n.size(); ++i)
            if(input_n[i].first <= 0)
                v.push_back(i);
        return v;
    }

    /// get list of ``free'' inputs with no wait threshold
    std::vector<size_t> get_free() const {
        std::vector<size_t> v;
        for(size_t i=0; i<input_n.size(); ++i)
            if(input_n[i].second < 0)
                v.push_back(i);
        return v;
    }

    /// stop waiting on any "stuck" inputs
    std::vector<size_t> unstick() {
        auto v = get_waiting();
        for(auto nI: v) set_required(nI,-1);
        return v;
    }

protected:

    /// pop next element (and push to nextSink)
    void pop() {
        auto& o = PQ.top();
        if(!--input_n[o.first].first) ++inputs_waiting;
        if(nextSink) nextSink->push(o.second);
        PQ.pop();
    }

    /// number of inputs with input_n.first <= 0
    int inputs_waiting = 0;
    /// counter for (required number, waiting threshold) of datapoints from each input
    std::vector<std::pair<int,int>> input_n;

    /// one item from enumerated source
    struct iT: public std::pair<size_t,Tmut_t> {
        /// reverse ordering operator
        bool operator<(const iT& b) const {
            return ordering_t(deref_if_ptr(b.second)) < ordering_t(deref_if_ptr(this->second));
        }
    };

    /// ordered inputs
    std::priority_queue<iT, std::vector<iT>> PQ;
};

#endif
