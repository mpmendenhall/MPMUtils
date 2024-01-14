/// @file PDGid.hh Particle Data Group particle ID numbers
// Michael P. Mendenhall, LLNL 2022

#ifndef PDGID_HH
#define PDGID_HH

#include <string>
using std::string;
#include <inttypes.h>

/// PDG / Geant4 particle type identifiers
enum PDGid_t: int32_t {
    PDG_X           = 0,
    PDG_ELECTRON    = 11,
    PDG_POSITRON    = -PDG_ELECTRON,
    PDG_NUE         = 12,
    PDG_NUEBAR      = -PDG_NUE,
    PDG_MUMINUS     = 13,
    PDG_MUPLUS      = -PDG_MUMINUS,
    PDG_GAMMA       = 22,
    PDG_PI0         = 111,
    PDG_PIPLUS      = 211,
    PDG_PIMINUS     = -PDG_PIPLUS,
    PDG_K0          = 311,
    PDG_KPLUS       = 321,
    PDG_KMINUS      = -PDG_KPLUS,
    PDG_NEUTRON     = 2112,
    PDG_PROTON      = 2212,
    // reserved "For MC internal use" range 81--100
    PDG_OPTICALPHOTON = 81,
    // ions
    PDG_ION_A       =         10,
    PDG_ION_Z       =      10000,
    PDG_ION         = 1000000000,
    PDG_TRITON      = PDG_ION + PDG_ION_Z + 3*PDG_ION_A,
    PDG_ALPHA       = PDG_ION + 2*PDG_ION_Z + 4*PDG_ION_A
};

/// element symbols; "n" for Z=0, "!" for Z > 118
const string& elSymb(unsigned int Z);

/// identifier for an ion
constexpr PDGid_t PDG_IonZA(int Z, int A, int L = 0) { return PDGid_t(PDG_ION + PDG_ION_Z*Z + PDG_ION_A*A + L); }

/// PDG/Geant4 particle type to "human-readable" name
string PDG_PID_name(PDGid_t i);

/// display name for isotope A, Z
string isot_name_ZA(unsigned int Z, unsigned int A);

#endif
