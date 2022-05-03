/// \file NullSink.hh Ignore datastream contents
// -- Michael P. Mendenhall, LLNL 2022

#ifndef NULLSINK_HH
#define NULLSINK_HH

#include "DataSink.hh"
#include "ConfigFactory.hh"
#include "XMLTag.hh"

/// Ignore each received object
template<class T>
class NullSink: public DataSink<T>, public XMLProvider  {
public:
    /// Constructor
    explicit NullSink(const Setting&): XMLProvider("NullSink") { }
    /// Do nothing!
    void push(T&) override { }
};

#endif
