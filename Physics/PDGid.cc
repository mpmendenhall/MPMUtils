/// \file PDGid.cc

#include "PDGid.hh"
#include "to_str.hh"

string isot_name(unsigned int A, unsigned int Z) {
    const size_t maxel = 19;
    static const char* const elnames[maxel+1] = {"?","H","He","Li","Be","B","C","N","O","F","Ne","Na","Mg","Al","Si","P","S","Cl","Ar","K"};
    if(A==1 && Z==0) return "n";
    return (Z<=maxel)? to_str(A)+elnames[Z] : "A"+to_str(A)+"Z"+to_str(Z);
}

string PDG_PID_name(PDGid_t i) {
    switch(i) {
        case PDG_ALPHA:    return "alpha";
        case PDG_TRITON:   return "triton";
        case PDG_ELECTRON: return "e-";
        case PDG_POSITRON: return "e+";
        case PDG_MUMINUS:  return "mu-";
        case PDG_MUPLUS:   return "mu+";
        case PDG_GAMMA:    return "gamma";
        case PDG_NEUTRON:  return "n";
        case PDG_PROTON:   return "p";
        default: break;
    }

    if(i > PDG_ION) {
        auto A =  (i % PDG_ION_Z)/PDG_ION_A;
        auto A1 = i % PDG_ION_A;
        auto Z = (i % 1000000)/PDG_ION_Z;
        string nm = isot_name(A,Z);
        return A1? nm + "-" + to_str(A1) : nm;
    }
    else return "PID:"+to_str(i);
}
