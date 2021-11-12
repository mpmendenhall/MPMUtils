/// \file MCTAL_File.hh Parser utilities for MCNP "MCTAL" format files
// Michael P. Mendenhall, LLNL 2021

#ifndef MCTAL_FILE_HH
#define MCTAL_FILE_HH

#include "MCTAL_Header.hh"
#include "MCTAL_Tally.hh"

/// Parser utilities for MCNP "MCTAL" format files
class MCTAL_File: public vector<MCTAL_Tally> {
public:
    /// Constructor
    explicit MCTAL_File(istream& i);
    /// print summary to stdout
    void display() const;
protected:
    lineReader lr;      ///< line-by-line reader
public:
    MCTAL_Header hdr;   ///< file header
};

#endif
