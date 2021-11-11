--------------------------------------------------------------------------------
Utilities for parsing MCNP tally output "MCTAL"-format files
based on section 5.3.4 of the MCNP 6.2 User's Manual, LA-UR-17-29981
-- Michael P. Mendenhall, LLNL 2021
--------------------------------------------------------------------------------

MCNP can be set to output tallies to a MCTAL-format (ASCII tables) file by
specifying `mctal=<filename>` as a command-line argument to `mcnp6`, and
setting the `mct` option on the PRDMP card (see section 3.3.7.2.3 in the
MCNP 6.2 user manual).

--------------------------------------------------------------------------------

To use this parser, compile the `MCTAL_*` source files (plus `to_str.hh`) into
your own code, and:

#include "MCTAL_File.hh"
#include <fstream>
std::ifstream i("path to MCTAL file...");
MCTAL_File MF(i);

loads the contents of the file into a `MCTAL_File` object `MF`, which is a
`vector<MCTAL_Tally>` listing each tally in the file.

Each `MCTAL_Tally` is a `vector<valerr_t>`, the (flattened) tally data array of
bin contents and errors, along with info on the binning of the data, in an
up-to-8-dimensional array along some subset of MCNP's 8 binning axis options:
    * "F": cell, surface, or detector bin numbers
    * "D": "number of total vs. direct or flagged vs. unflagged bins"
    * "U": user-defined bin numbers
    * "S": segment bin numbers
    * "M": multiplier option bins
    * "C": cosine relative to surface normal angular bins
    * "E": energy bins (in MeV)
    * "T": time bins (in shakes = 1e-8 s)

--------------------------------------------------------------------------------

1D or 2D tallies (or a slice of a higher-dimensional tally) can be converted to
ROOT TH1D/TH2D histograms (with error bars) by:

#include "MCTAL_toROOT.hh"
TH1D* h1 = tallyH1("tally_histogram_name",
                   "histogram description",
                   MF.at(<position in tallies vector>));
TH2D* h2 = tallyH2("tally_histogram_name",
                   "histogram description",
                   MF.at(<position in tallies vector>));
where the axes are automatically selected from the "active" axes of the tally,
e.g. those with more than 1 bin.

--------------------------------------------------------------------------------

To select a different set of axes (if there are more available than those
selected by the defaults), an additional optional argument can be passed to
tallyH1 (or two to tallyH2) selected from:

/// axis identifiers
enum tallyax_id_t {
    AXIS_T = 0,
    AXIS_E = 1,
    AXIS_C = 2,
    AXIS_M = 3,
    AXIS_S = 4,
    AXIS_U = 5,
    AXIS_D = 6,
    AXIS_F = 7
};

which may be followed by a starting offset to select a slice along other axes:

auto h = tallyH1("h",
                 "slice along time axis in second F (cell) bin",
                 MF.at(2),
                 AXIS_T,
                 MF.at(2).axis(AXIS_F).stride);

loads a 1D time axis histogram for the second F (cell number) bin.
