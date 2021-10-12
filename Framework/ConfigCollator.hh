/// \file ConfigCollator.hh Configturation-buildable Collator object
// Michael P. Mendenhall, LLNL 2021

#ifndef CONFIGCOLLATOR_HH
#define CONFIGCOLLATOR_HH

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

#include "Collator.hh"

/// Configturation-buildable Collator object
template<class T>
class ConfigCollator: public _ConfigCollator, public Collator<T> {
public:
    /// inherit constructor
    using _ConfigCollator::_ConfigCollator;
};

/// Registration in AnaIndex
template<typename T>
_ConfigCollator* AnaIndex<T>::makeConfigCollator(const Setting& S) const { return new ConfigCollator<T>(S); }

#endif
