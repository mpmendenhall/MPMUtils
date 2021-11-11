/// \file MCTAL_Header.hh Parser for MCTAL file header
// Michael P. Mendenhall, LLNL 2021

#ifndef MCTAL_HEADER_HH
#define MCTAL_HEADE

#include "MCTAL_Includes.hh"
#include <cstdint>

/// Header line in MCTAL file
class MCTAL_Header {
public:
    /// Constructor
    explicit MCTAL_Header(istream& i);

    string kod;         ///< name of the code, e.g. "MCNP6"
    string ver;         ///< code version, e.g. "6.2"
    string prob_date;   ///< "problem ID" date
    string prob_time;   ///< "problem ID" time
    string probid;      ///< "problem ID" description
    int knod = 0;       ///< "dump number"
    int64_t nps = 0;    ///< number of particle histories run
    int64_t rnr = 0;    ///< number of pseudorandom numbers used

    int ntal = 0;       ///< number of tallies in file
    int npert = 0;      ///< number of perturbations
    vector<int> tallynums;  ///< tally ID numbers

    /// print summary to stdout
    void display() const;
};

#endif
