/// \file CumulativeData.hh Base class for objects that can be scaled/summed
// -- Michael P. Mendenhall, LLNL 2020

#ifndef CUMULATIVEDATA_HH
#define CUMULATIVEDATA_HH

#include <string>
using std::string;
#include <stdio.h>

/// Base class for objects that can be scaled/summed
class CumulativeData {
public:
    /// Constructor
    explicit CumulativeData(const string& _name = ""): name(_name) { }
    /// Polymorphic Destructor
    virtual ~CumulativeData() { }

    /// Scale contents by factor
    virtual void Scale(double s) = 0;
    /// Clear contents
    virtual void ClearCumulative() { Scale(0); }
    /// Add another of same type, with scale factor
    virtual void Add(const CumulativeData& CD, double s = 1.) = 0;
    /// End-of-fill notification
    virtual void endFill() { }
    /// Store state
    virtual void Write() { }

    /// inline sum
    CumulativeData& operator+=(const CumulativeData& rhs) { Add(rhs); return *this; }
    /// inline product
    CumulativeData& operator*=(double s) { Scale(s); return *this; }

    /// debugging contents print
    virtual void display() const { printf("CumulativeData '%s'\n", name.c_str()); }

    string name;            ///< savefile name
    bool scalable = true;   ///< whether scaling should be applied
};

#endif
