/// \file DataSink.hh Base class for accepting a stream of objects
// -- Michael P. Mendenhall, LLNL 2019

#ifndef DATASINK_HH
#define DATASINK_HH

#include "SignalSink.hh"

/// Virtual base class for accepting a stream of objects
template<typename T>
class DataSink: virtual public SignalSink {
public:
    /// received data type
    typedef T sink_t;
    /// mutable variant of received data type
    typedef typename std::remove_const<T>::type mutsink_t;
    /// take instance of object
    virtual void push(sink_t&) = 0;
};

#endif
