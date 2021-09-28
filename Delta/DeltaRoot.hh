/// \file DeltaRoot.hh summarize ROOT file differences
// Michael P. Mendenhall, LLNL 2021

#include "DeltaBase.hh"
#include <TFile.h>

/// summarize ROOT file differences
class DeltaRoot: public DeltaBase {
public:
    /// type-specific comparison
    bool _compare() override;

protected:
    /// recursive TDirectory contents comparison
    bool tdcompare(TDirectory* d1, TDirectory* d2) const;
};
