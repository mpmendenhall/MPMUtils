/// \file _Collator.hh Un-typed base Collator
// -- Michael P. Mendenhall, LLNL 2021

#ifndef _COLLATOR_HH
#define _COLLATOR_HH

#include "_DataSink.hh"
#include "Threadworker.hh"

#include <cstddef> // for size_t on some systems
#include <vector>
using std::vector;
#include <utility> // for std::pair

/// Type-independent re-casting base
class _Collator: public Threadworker, virtual public _SinkUser, public SignalSink {
public:
    /// add enumerated input slot (return enumeration number)
    size_t add_input(int nreq = 0);
    /// connect SinkUser as input
    virtual void connect_input(_SinkUser&, int /* nreq */ = 0) {
        throw std::logic_error("Type-specific subclass required to connect inputs");
    }

    /// change minimum number required from input
    void change_required(size_t nI, int i);
    /// get requirement threshold for input
    int get_required(size_t nI) const { return input_n.at(nI).second; }
    /// set minimum required from input
    void set_required(size_t nI, int i) { change_required(nI, i-get_required(nI)); }

    /// clear all inputs
    virtual void reset();
    /// get list of ``waiting'' inputs
    vector<size_t> get_waiting() const;
    /// get list of ``free'' inputs with no wait threshold
    vector<size_t> get_free() const;
    /// stop waiting on any "stuck" inputs
    vector<size_t> unstick();

    /// ignore signals
    void signal(datastream_signal_t) override { }

protected:
    int inputs_waiting = 0; ///< number of inputs with input_n.first <= 0

    /// counter for (required number, waiting threshold) of datapoints from each input
    vector<std::pair<int,int>> input_n;
};

#endif
