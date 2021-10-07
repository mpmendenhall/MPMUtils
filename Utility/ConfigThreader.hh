/// \file ConfigThreader.hh wrapper to run Configurable in its own thread
// Michael P. Mendenhall, LLNL 2021

#ifndef CONFIGTHREADER_HH
#define CONFIGTHREADER_HH

#include "ConfigFactory.hh"
#include "Threadworker.hh"

/// wrapper to run Configurable in its own thread
class ConfigThreader: public Threadworker {
public:
    /// Constructor
    ConfigThreader(Configurable& _C): C(_C) { }
    /// run in thread
    void threadjob() override { C.run(); }
    /// the Configurable to run
    Configurable& C;
};

#endif
