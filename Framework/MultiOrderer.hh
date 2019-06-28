/// \file MultiOrderer.hh Order items received from multiple "push" sources

#ifndef MULTIORDERER_HH
#define MULTIORDERER_HH

#include "DataSink.hh"
#include "deref_if_ptr.hh"

#include <queue>
using std::priority_queue;
#include <vector>
using std::vector;
#include <utility>
using std::pair;
#include <cassert>

/// Order items received from multiple "push" sources
template<class T0, typename ordering_t = double>
class MultiOrderer {
protected:
    /// item from enumerated source
    typedef pair<size_t,T0> iT0;

public:
    /// polymorphic destructor; remember to flush before!
    virtual ~MultiOrderer() { assert(PQ.empty()); }

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
        if(n <= 0 && n+i > 0) { --inputs_waiting; assert(inputs_waiting >= 0); }
        if(n > 0 && n+i <= 0) ++inputs_waiting;
        input_n[nI].second += i;
        n += i;
    }
    /// get requirement threshold for input
    int get_required(size_t nI) const { return input_n.at(nI).second; }
    /// set minimum required from input
    void set_required(size_t nI, int i) { change_required(nI, i-get_required(nI)); }

    /// convenience input handle for this orderer
    class MOInput: public DataSink<T0> {
    public:
        /// constructor
        MOInput(MultiOrderer& _M): M(_M), n(M.add_input()) { }
        /// DataSink push
        void push(const T0& o) override { M.push(n,o); }
        /// bulk push
        void push(const vector<T0>& os) { M.push(n,os); }

    protected:
        MultiOrderer& M;    ///< orderer
        size_t n;           ///< input enumeration
    };

    /// add item from enumerated input
    void push(size_t nI, const T0& o) {
        assert(nI < input_n.size());
        auto& n = input_n[nI].first;
        if(!n++) { --inputs_waiting; assert(inputs_waiting >= 0); }
        PQ.emplace(nI,o);
        while(!inputs_waiting && !PQ.empty()) pop();
    }

    /// bulk-add items
    void push(size_t nI, const vector<T0>& os) {
        assert(nI < input_n.size());
        if(!os.size()) return;
        auto& n = input_n[nI].first;
        if(n <= 0 && n + os.size() > 0) { --inputs_waiting; assert(inputs_waiting >= 0); }
        n += os.size();
        for(auto& o: os) PQ.emplace(nI,o);
        while(!inputs_waiting && !PQ.empty()) pop();
    }

    /// flush all data
    virtual void flush() {
        while(!PQ.empty()) pop();
        if(nextSink) nextSink->flush();
    }

    /// clear all inputs
    virtual void reset() {
        flush();
        inputs_waiting = 0;
        input_n.clear();
    }

    /// get list of ``waiting'' inputs
    vector<size_t> get_waiting() const {
        vector<size_t> v;
        for(size_t i=0; i<input_n.size(); ++i)
            if(input_n[i].first <= 0)
                v.push_back(i);
        return v;
    }

    /// get list of ``free'' inputs with no wait threshold
    vector<size_t> get_free() const {
        vector<size_t> v;
        for(size_t i=0; i<input_n.size(); ++i)
            if(input_n[i].second < 0)
                v.push_back(i);
        return v;
    }

    /// stop waiting on any "stuck" inputs
    vector<size_t> unstick() {
        auto v = get_waiting();
        for(auto nI: v) set_required(nI,-1);
        return v;
    }

    DataSink<T0>* nextSink = nullptr;   ///< destination for ordered items

protected:

    /// pop next element
    void pop() {
        auto& o = PQ.top();
        if(!--input_n[o.first].first) ++inputs_waiting;
        if(nextSink) nextSink->push(o.second);
        PQ.pop();
    }

    int inputs_waiting = 0;         ///< number of inputs with input_n <= 0
    vector<pair<int,int>> input_n;  ///< counter for required numbers of datapoints from each input, and "waiting" threhsold

    /// ordering comparator
    struct s_order {
        /// reverse ordering comparison
        bool operator()(const iT0& a, const iT0& b) const {
            return ordering_t(deref_if_ptr(b.second)) < ordering_t(deref_if_ptr(a.second));
        }
    };

    /// ordered inputs
    priority_queue<iT0, vector<iT0>, s_order> PQ;
};

#endif
