/// \file DataSink.hh Base class for accepting a stream of objects
// Michael P. Mendenhall, LLNL 2019

#ifndef DATASINK_HH
#define DATASINK_HH

/// Virtual base class for accepting a stream of objects
template<class C>
class DataSink {
public:
    /// take instance of object
    virtual void push(const C&) = 0;
    /// flush processing
    virtual void flush() { }
};

/// Virtual base for chaining to next sink
template<class C>
class SinkOut {
public:
    virtual void setSink(DataSink<C>* ns) { nextSink = ns; }
protected:
    DataSink<C>* nextSink = nullptr;
};

#endif
