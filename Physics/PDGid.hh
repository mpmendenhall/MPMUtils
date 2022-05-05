/// \file PDGid.hh Particle Data Group particle ID numbers
// Michael P. Mendenhall, LLNL 2022

#ifndef PDGID_HH
#define PDGID_HH

#include <string>
using std::string;
#include <inttypes.h>

/// PDG / Geant4 particle type identifiers
/// Special IDs for particles without PDG encoding
/// using reserved "For MC internal use" range 81--100
enum PDGid_t: int32_t {
    PDG_X        = 0,
    PDG_ELECTRON = 11,
    PDG_POSITRON = -11,
    PDG_NUEBAR   = -12,
    PDG_MUMINUS  = 13,
    PDG_MUPLUS   = -13,
    PDG_GAMMA    = 22,
    PDG_NEUTRON  = 2112,
    PDG_PROTON   = 2212,
    PDG_OPTICALPHOTON = 81,
    PDG_ION_A    = 10,
    PDG_ION_Z    = 10000,
    PDG_ION      = 1000000000,
    PDG_TRITON   = PDG_ION + PDG_ION_Z + 3*PDG_ION_A,
    PDG_ALPHA    = PDG_ION + 2*PDG_ION_Z + 4*PDG_ION_A
};

/// identifier for an ion
inline PDGid_t PDG_Ion(int Z, int A, int L = 0) { return PDGid_t(PDG_ION + PDG_ION_Z*Z + PDG_ION_A*A + L); }

/// PDG/Geant4 particle type to "human-readable" name
string PDG_PID_name(PDGid_t i);

/// display name for isotope A, Z
string isot_name(unsigned int A, unsigned int Z);

#endif
