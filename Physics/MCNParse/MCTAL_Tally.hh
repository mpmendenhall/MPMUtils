/// \file MCTAL_Tally.hh Parser utilities for MCNP "MCTAL" file tallies
// Michael P. Mendenhall, LLNL 2021

#ifndef MCTAL_TALLY_HH
#define MCTAL_TALLY_HH

#include "MCTAL_Axis.hh"
#include "MCTAL_Aux.hh"

/// value and relative error
struct valerr_t {
    double val;     ///< bin value
    double rel_err; ///< fractional relative error
};

/// one Tally table in an MCTAL file
class MCTAL_Tally: public vector<valerr_t> {
public:
    /// Constructor
    MCTAL_Tally(lineReader* i = nullptr);

    /// read from file
    void load(lineReader& i);

    /// get particle type name
    static string ptype_name(ptype_t p);

    /// tally type identifier
    enum tally_t {
        TALLY_NONE    = 0,  ///< undefined
        TALLY_CURRENT = 1,  ///< surface current
        TALLY_SFLUX   = 2,  ///< surface flux
        TALLY_UNUSED  = 3,  ///< (unused F3 tally)
        TALLY_CFLUX   = 4,  ///< track-length estimate of cell flux
        TALLY_PFLUX   = 5,  ///< point flux
        TALLY_EDEP    = 6,  ///< track-length estimate of energy deposition
        TALLY_EFIS    = 7,  ///< track-length estimate of fission E deposition
        TALLY_PULSE   = 8   ///< Energy distribution of detected pulses
    } tally;

    /// names for tally identifiers
    static const char* tally_name(tally_t t);

    /// detector type identifier
    enum detector_t {
        DET_NONE    = 0,    ///< no detector
        DET_POINT   = 1,    ///< point detector
        DET_RING    = 2,    ///< ring detector
        DET_PINHOLE = 3,    ///< pinhole radiograph (FIP)
        DET_FIR     = 4,    ///< rectangular transmitted image radiograph (FIR)
        DET_FIC     = 5,    ///< cylindrical transmitted image radiograph (FIC)
    } detector;

    /// names for detector identifiers
    static const char* detector_name(detector_t t);

    /// tally modifier
    enum tallymod_t {
        TALLYMOD_NONE = 0
    } tallymod;

    /// print summary info to stdout
    void display() const;

    /// array access indexed on active axes
    const valerr_t&
    operator()(size_t a1,   size_t a2=0, size_t a3=0, size_t a4=0,
               size_t a5=0, size_t a6=0, size_t a7=02, size_t a8=0) const;

    int probnum = 0;    ///< tally problem name identifier
    set<ptype_t> ptype; ///< particle types
    vector<tallyax_id_t> axes;  ///< active axes enumeration

    /// get enumerated axis
    const MCTAL_Axis& axis(tallyax_id_t a) const;
    /// get mutable enumerated axis
    MCTAL_Axis&
    _axis(tallyax_id_t a) { return const_cast<MCTAL_Axis&>(axis(a)); }

    MCTAL_IntAx  Fbins; ///< cell, surface, or detector bin numbers
    MCTAL_Axis   Dbins; ///< total vs. direct or flagged vs. unflagged bins
    MCTAL_Axis   Ubins; ///< user bins
    MCTAL_Axis   Sbins; ///< segment bins
    MCTAL_Axis   Mbins; ///< multiplier bins
    MCTAL_AxBins Cbins; ///< cosine bins
    MCTAL_AxBins Ebins; ///< energy bins [MeV]
    MCTAL_AxBins Tbins; ///< time bins [shakes = 1e-8 s]

    MCTAL_TFC tfc;      ///< Tally Fluctuation Table
    MCTAL_KCODE kcyc;   ///< Code Cycles info
};

#endif
