/// \file MCTAL_toROOT.hh Convert MCTAL Tallies to ROOT histograms
// Michael P. Mendenhall, LLNL 2021

#ifndef MCTAL_TOROOT_HH
#define MCTAL_TOROOT_HH

#include <TH1D.h>
#include <TH2D.h>
#include "MCTAL_Tally.hh"

/// extract TH1D from tally
TH1D* tallyH1(const string& name, const string& title,
              const MCTAL_Tally& t, tallyax_id_t a = AXIS_END, size_t i0 = 0);

/// extract TH2D from tally
TH2D* tallyH2(const string& name, const string& title,
              const MCTAL_Tally& t, tallyax_id_t a1 = AXIS_END,
              tallyax_id_t a2 = AXIS_END, size_t i0 = 0);

#endif
