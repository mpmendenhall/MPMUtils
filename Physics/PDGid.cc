/// \file PDGid.cc

#include "PDGid.hh"
#include "to_str.hh"

const string& elSymb(unsigned int Z) {
    const size_t maxel = 119;
    static const string elnames[maxel] = {       "n",
        "H",                                                                                    "He",
        "Li","Be",                                                     "B", "C", "N", "O", "F", "Ne",
        "Na","Mg",                                                     "Al","Si","P", "S", "Cl","Ar",
        "K", "Ca","Sc",   "Ti","V", "Cr","Mn","Fe","Co","Ni","Cu","Zn","Ga","Ge","As","Se","Br","Kr",
        "Rb","Sr","Y",    "Zr","Nb","Mo","Tc","Ru","Rh","Pd","Ag","Cd","In","Sn","Sb","Te","I", "Xe",
        "Cs","Ba","La",
                       "Ce","Pr","Nd","Pm","Sm","Eu","Gd","Tb","Dy","Ho","Er","Tm","Yb","Lu",
                          "Hf","Ta","W", "Re","Os","Ir","Pt","Au","Hg","Tl","Pb","Bi","Po","At","Rn",
        "Fr","Ra","Ac",
                       "Th","Pa","U", "Np","Pu","Am","Cm","Bk","Cf","Es","Fm","Md","No","Lr",
                          "Rf","Db","Sg","Bh","Hs","Mt","Ds","Rg","Cn","Nh","Fl","Mc","Lv","Ts","Og"
    };
    static const string whoknows = "!";
    return Z < maxel? elnames[Z] : whoknows;
}

string isot_name_ZA(unsigned int Z, unsigned int A) {
    if(A==1 && Z==0) return "n";
    return to_str(A)+elSymb(Z);
}

string PDG_PID_name(PDGid_t i) {
    switch(i) {
        case PDG_X:         return "???";
        case PDG_ALPHA:     return "alpha";
        case PDG_TRITON:    return "triton";
        case PDG_ELECTRON:  return "e-";
        case PDG_POSITRON:  return "e+";
        case PDG_NUEBAR:    return "antinu_e";
        case PDG_MUMINUS:   return "mu-";
        case PDG_MUPLUS:    return "mu+";
        case PDG_GAMMA:     return "gamma";
        case PDG_NEUTRON:   return "n";
        case PDG_PROTON:    return "p";
        case PDG_PI0:       return "pi0";
        case PDG_PIPLUS:    return "pi+";
        case PDG_PIMINUS:   return "pi-";
        case PDG_K0:        return "K0";
        case PDG_KPLUS:     return "K+";
        case PDG_KMINUS:    return "K-";
        default: break;
    }

    if(i > PDG_ION) {
        auto A =  (i % PDG_ION_Z)/PDG_ION_A;
        auto A1 = i % PDG_ION_A;
        auto Z = (i % 1000000)/PDG_ION_Z;
        string nm = isot_name_ZA(Z,A);
        return A1? nm + "-" + to_str(A1) : nm;
    }
    else return "PID:"+to_str(i);
}
