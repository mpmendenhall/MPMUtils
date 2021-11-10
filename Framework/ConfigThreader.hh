/// \file ConfigThreader.hh wrapper to run Configurable in its own thread
// Michael P. Mendenhall, LLNL 2021

#ifndef CONFIGTHREADER_HH
#define CONFIGTHREADER_HH

#include "ConfigFactory.hh"
#include "Threadworker.hh"
#include "XMLTag.hh"

/// combine Configurable with Threadworker
class ConfigThreader: public Configurable, public Threadworker, virtual public XMLProvider {
public:
    /// Constructor
    explicit ConfigThreader(const Setting& S, int i = 0):
    XMLProvider("ConfigThreader"), Configurable(S), Threadworker(i) { }
    /// run in thread
    void threadjob() override { run(); }
};

/// wrapper to run Configurable in its own thread
class ConfigThreadWrapper: public Threadworker {
public:
    /// Constructor
    explicit ConfigThreadWrapper(Configurable* _C = nullptr, int i = 0): Threadworker(i), C(_C) { }
    /// Destructor
    ~ConfigThreadWrapper() { if(checkRunning()) finish_mythread(); if(ownsWrapped) delete C; }
    /// run in thread
    void threadjob() override { if(C) C->run(); }
    /// the Configurable to run
    Configurable* C;
    /// ownership to delete C
    bool ownsWrapped = true;
};

#endif
