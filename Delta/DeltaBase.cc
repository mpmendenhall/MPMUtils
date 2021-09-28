/// DeltaBase.cc
// Michael P. Mendenhall, LLNL 2021

#include "DeltaBase.hh"
#include "PathUtils.hh"
#include "DeltaDiff.hh"
#include "DeltaRoot.hh"
#include "StringManip.hh"

DeltaBase::CompareType_t DeltaBase::inferType() {
    comptype = COMPARE_DIFF;

    bool da = dirExists(fref);
    bool db = dirExists(fcomp);
    if(da != db) return comptype;
    if(da && db) return (comptype = COMPARE_DIR);

    if(!fileExists(fref)) throw std::runtime_error("Reference file '" + fref + "' not found");
    if(!fileExists(fcomp)) throw std::runtime_error("Coparison file '" + fcomp + "' not found");
    if(suffix(fref) == ".root" && suffix(fcomp) == ".root") return (comptype = COMPARE_ROOT);

    return comptype;
}

template<class DD>
bool ddcomp(const DeltaBase& B) {
    DD D;
    (DeltaBase&)D = B;
    return D._compare();
}

bool DeltaBase::compare() const {
    bool c = false;
    if(comptype == COMPARE_DIFF || comptype == COMPARE_DIR) c = ddcomp<DeltaDiff>(*this);
    else if(comptype == COMPARE_ROOT) c = ddcomp<DeltaRoot>(*this);
    else throw std::runtime_error("Unimplemented comparison type requested");

    if(c) printf("Inputs compared equivalent.\n");
    else printf("Inputs are different.\n");
    return c;
}
