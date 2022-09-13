/// DeltaBase.hh Base class for file delta operations
// Michael P. Mendenhall, LLNL 2021

#ifndef DELTABASE_HH
#define DELTABASE_HH

#include <string>
using std::string;
#include <stdexcept>

/// Base class for file delta operations
class DeltaBase {
public:
    /// Constructor
    explicit DeltaBase(const string& f1 = "", const string& f2 = ""): fref(f1), fcomp(f2) { }
    /// polymorphic destructor
    virtual ~DeltaBase() { }

    /// comparison mode selection
    enum CompareType_t {
        Diff,   ///< compare using "diff"
        Dir,    ///< directories/files comparison
        ROOT    ///< ROOT file comparison
    } comptype = CompareType_t::Diff;

    /// infer comparison type from input files
    CompareType_t inferType();
    /// perform comparison for appropriate type
    bool compare() const;

    string fref;    ///< reference input name
    string fcomp;   ///< comparison input name
    string outdir = "./delta/"; ///< output directory

    /// type-specific comparison
    virtual bool _compare() { return false; }
};

#endif
