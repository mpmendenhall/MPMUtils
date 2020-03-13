/// \file CumulativeData.hh Base class for objects that can be scaled/summed
// -- Michael P. Mendenhall, LLNL 2020

#ifndef CUMULATIVEDATA_HH
#define CUMULATIVEDATA_HH

#include <string>
using std::string;

/// Base class for objects that can be scaled/summed
class CumulativeData {
public:
    /// Constructor
    CumulativeData(const string& _name): name(_name) { }
    /// Polymorphic Destructor
    virtual ~CumulativeData() { }

    /// Scale contents by factor
    virtual void Scale(double s) = 0;
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

    const string name;  ///< savefile name
};

#endif
