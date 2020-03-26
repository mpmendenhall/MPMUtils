/// \file NoCopy.hh Mixin class to prevent copy construction or assignment
// -- Michael P. Mendenhall, LLNL 2020

#ifndef NOCOPY_HH
#define NOCOPY_HH

/// Mixin class to prevent copy construction or assignment
class NoCopy {
public:
    /// Default construction OK
    NoCopy() { }
    /// forbid copy construction
    NoCopy(const NoCopy&) = delete;
    /// forbid copy assignment
    const NoCopy& operator=(const NoCopy&) = delete;
};

#endif
