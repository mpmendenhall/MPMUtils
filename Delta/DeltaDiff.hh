/// @file DeltaDiff.hh gnu diff based delta
// Michael P. Mendenhall, LLNL 2021

#include "DeltaBase.hh"

/// gnu diff based delta
class DeltaDiff: public DeltaBase {
public:
    /// type-specific comparison
    bool _compare() override;
};
