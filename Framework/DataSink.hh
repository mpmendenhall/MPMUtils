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

#endif
