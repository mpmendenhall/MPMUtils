/// \file MCTAL_Aux.hh Parser utilities for MCNP "MCTAL" file tallies
// Michael P. Mendenhall, LLNL 2021

#ifndef MCTAL_AUX_HH
#define MCTAL_AUX_HH

#include "MCTAL_Includes.hh"

/// Tally Fluctuation Chart bin entry
struct s_tfc {
    int nps;        ///< number of particles
    double tally;   ///< tally value
    double err;     ///< error
    double fom;     ///< figure of merit
};

/// TFC Tally Fluctuation Chart
class MCTAL_TFC: public vector<s_tfc> {
public:
    /// read from file
    void load(lineReader& i);

    int j_tf[8];    ///< bin indexes of the tally fluctuation chart bin

    /// print summary info to stdout
    void display() const;
};

/// code cycle info
struct s_kcyc {
    double keff1 = 0, keff2 = 0, keff3 = 0;
    double rl1 = 0, rl2 = 0;
};

/// KCODE code cycles table
class MCTAL_KCODE: public vector<s_kcyc> {
public:
    /// read from file
    void load(lineReader& i);

    int n_cyc = 0;  ///< number of code cycles
    int n_scyc = 0; ///< number of settle cycles
    int n_var = 0;  ///< number of variables for each cycle

    /// print summary info to stdout
    void display() const;
};

#endif
