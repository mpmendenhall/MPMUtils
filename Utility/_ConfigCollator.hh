/// \file _ConfigCollator.hh Configturation-buildable Collator object base
// Michael P. Mendenhall, LLNL 2021

#ifndef _CONFIGCOLLATOR_HH
#define _CONFIGCOLLATOR_HH

#include "_Collator.hh"
#include "ConfigThreader.hh"
#include "GlobalArgs.hh"
#include "XMLTag.hh"
#include <thread>

/// Type-independent re-casting base
class _ConfigCollator: public Configurable, public XMLProvider, virtual public _Collator {
public:
    /// Constructor
    explicit _ConfigCollator(const Setting& S):
    Configurable(S), XMLProvider("Collator"), nthreads(std::thread::hardware_concurrency()) {
        S.lookupValue("nthreads", nthreads);
        optionalGlobalArg("nParallel", nthreads, "number of parallel collated processes (0 for single-threaded)");
    }

    /// Destructor
    ~_ConfigCollator() { delete C0; }

    int nthreads;               ///< number of separate input threads (0 for single-threaded)
    Configurable* C0 = nullptr; ///< representative input chain head

    /// XML output info
    void _makeXML(XMLTag& X) override { X.addAttr("nparallel", nthreads); }

    /// Run as top-level object
    void run() override;
    /// single-threaded run mode (bypass collation, works in any class)
    void run_singlethread();
    /// multi-threaded collating mode (only works in type-specific subclass!)
    void run_multithread();
};

#endif
